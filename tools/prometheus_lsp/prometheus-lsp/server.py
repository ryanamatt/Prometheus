"""
Prometheus Language Server
==========================
Implements the Language Server Protocol for the Prometheus (.prm) language.

Features
--------
* Diagnostics   – real-time lexer/parse-error squiggles via the Prometheus binary
* Completion     – keywords, types, builtins, user-defined symbols, list methods
* Hover          – docs for builtins, types, keywords, user symbols
* Go-to-def      – jump to variable / function declarations in the same file
* Document symbols – outline of functions and variables
* Semantic tokens – fine-grained token coloring
* Signature help  – parameter hints for builtins and user functions
* Formatting      – brace-aware auto-indent formatter
"""

from __future__ import annotations

import re
import logging
from typing import Optional
from pygls.lsp.server import LanguageServer
from lsprotocol import types

logging.basicConfig(level=logging.INFO,
                    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s")
logger = logging.getLogger("prometheus-lsp")

server = LanguageServer("prometheus-language-server", "v0.1.1-dev")

# ---------------------------------------------------------------------------
# Language constants
# ---------------------------------------------------------------------------
KEYWORDS = [
    "if", "elif", "else", "while", "for", "func", "return",
    "import", "use", "true", "false",
]

TYPES = ["int", "str", "double", "bool", "void", "list", "dict"]

BUILTINS = {
    "print": {
        "sig": "print(expr, ...)",
        "doc": "Prints one or more values space-separated with a newline. Returns the printed string.",
        "params": ["expr", "..."],
    },
    "input": {
        "sig": "input(prompt?)",
        "doc": "Reads a line from stdin. Optional `prompt` string is printed first. Returns `str`.",
        "params": ["prompt?"],
    },
    "range": {
        "sig": "range(stop) / range(start, stop) / range(start, stop, step)",
        "doc": "Returns a `list[int]` following Python-style range semantics.",
        "params": ["start", "stop", "step"],
    },
    "type": {
        "sig": "type(expr)",
        "doc": "Returns the runtime type name of `expr` as a `str` (e.g. `\"int\"`, `\"list[str]\"`).",
        "params": ["expr"],
    },
}

STDLIB_MODULES = {
    "math":   "Standard math library (sqrt, pow, abs, floor, ceil, …)",
    "random": "Random number generation (rand, randint, shuffle, …)",
    "time":   "Time utilities (now, sleep, …)",
}

LIST_METHODS = {
    "append": {"sig": "list.append(value)",         "doc": "Appends `value` to the end of the list."},
    "len":    {"sig": "list.len()",                  "doc": "Returns the number of elements as `int`."},
    "insert": {"sig": "list.insert(index, value)",   "doc": "Inserts `value` at position `index`."},
    "pop":    {"sig": "list.pop()",                  "doc": "Removes and returns the last element."},
    "remove": {"sig": "list.remove(value)",          "doc": "Removes the first occurrence of `value`."},
    "clear":  {"sig": "list.clear()",               "doc": "Removes all elements from the list."},
}

KEYWORD_DOCS = {
    "if":     "Conditional branch: `if (condition) { ... }`",
    "elif":   "Additional conditional branch: `elif (condition) { ... }`",
    "else":   "Fallback branch: `else { ... }`",
    "while":  "Loop while condition is true: `while (condition) { ... }`",
    "for":    "C-style or range-based for loop.",
    "func":   "Function declaration: `func <return_type> name(params) { ... }`",
    "return": "Return a value from a function: `return expr;`",
    "import": "Import a `.prm` file: `import path/to/file;`",
    "use":    "Use a standard-library module: `use math;`",
    "true":   "Boolean literal `true`.",
    "false":  "Boolean literal `false`.",
    "int":    "32-bit integer type.",
    "double": "64-bit floating-point type.",
    "str":    "String type (UTF-8 text).",
    "bool":   "Boolean type (`true` / `false`).",
    "void":   "Return type indicating no value.",
    "list":   "Generic list type: `list[int]`, `list[str]`, etc.",
    "dict":   "Generic dict type: `dict[str, int]`, dict[double, bool], etc."
}

# Token patterns for semantic highlighting (order matters)
_TOKEN_PATTERNS = [
    ("comment",   re.compile(r"#[^\n]*")),
    ("string",    re.compile(r'"(?:[^"\\]|\\.)*"')),
    ("number",    re.compile(r"\b\d+(?:\.\d+)?\b")),
    ("keyword",   re.compile(r"\b(?:if|elif|else|while|for|func|return|import|use|true|false)\b")),
    ("type",      re.compile(r"\b(?:int|double|str|bool|void|list|dict)\b")),
    ("builtin",   re.compile(r"\b(?:print|input|range|type)\b")),
    ("operator",  re.compile(r"\*\*|[+\-*/%]=?|[=!<>]=?|&&|\|\||!")),
    ("delimiter", re.compile(r"[(){}\[\].,;:]")),
    ("ident",     re.compile(r"\b[a-zA-Z_]\w*\b")),
]

# ---------------------------------------------------------------------------
# Document store
# ---------------------------------------------------------------------------
_documents: dict[str, str] = {}

def _get_text(uri: str) -> str:
    return _documents.get(uri, "")

# ---------------------------------------------------------------------------
# Symbol extraction
# ---------------------------------------------------------------------------
def _extract_symbols(text: str) -> dict:
    functions, variables = [], []
    func_re = re.compile(
        r"^\s*func\s+(int|double|str|bool|void|list(?:\[[^\]]+\])?)\s+(\w+)\s*\(([^)]*)\)"
    )
    var_re = re.compile(
        r"^\s*(int|double|str|bool|list(?:\[[^\]]+\])?)\s+(\w+)\s*="
    )
    for lineno, raw in enumerate(text.splitlines()):
        stripped = re.sub(r"#.*", "", raw)
        m = func_re.match(stripped)
        if m:
            ret, name, params_raw = m.group(1), m.group(2), m.group(3).strip()
            col = stripped.index(name)
            functions.append((name, lineno, col, params_raw, ret))
            continue
        m = var_re.match(stripped)
        if m:
            vtype, vname = m.group(1), m.group(2)
            col = stripped.index(vname)
            variables.append((vname, lineno, col, vtype))
    return {"functions": functions, "variables": variables}

def _word_at(text: str, line: int, col: int) -> str:
    lines = text.splitlines()
    if line >= len(lines): return ""
    row = lines[line]
    s = col
    while s > 0 and (row[s-1].isalnum() or row[s-1] == "_"): s -= 1
    e = col
    while e < len(row) and (row[e].isalnum() or row[e] == "_"): e += 1
    return row[s:e]

def _preceding_dot(text: str, line: int, col: int) -> bool:
    lines = text.splitlines()
    if line >= len(lines): return False
    row = lines[line]
    s = col
    while s > 0 and (row[s-1].isalnum() or row[s-1] == "_"): s -= 1
    return s > 0 and row[s-1] == "."

# ---------------------------------------------------------------------------
# Diagnostics
# ---------------------------------------------------------------------------
_LEX_ERROR_RE   = re.compile(r"Lexer Error: \[Line (\d+)\] (.+)")
_PARSE_ERROR_RE = re.compile(r"Parse Error: \[Line (\d+)\] (.+)")

def _find_prometheus_binary() -> Optional[str]:
    import shutil
    for candidate in ["prometheus", "./prometheus", "../prometheus"]:
        found = shutil.which(candidate)
        if found: return found
    return None

def _diag(line, s, e, msg, sev=types.DiagnosticSeverity.Error) -> types.Diagnostic:
    return types.Diagnostic(
        range=types.Range(start=types.Position(line=line, character=s),
                          end=types.Position(line=line, character=e)),
        message=msg, severity=sev, source="prometheus",
    )

def _heuristic_diagnostics(text: str) -> list[types.Diagnostic]:
    diags = []
    brace_stack, paren_stack = [], []
    for lineno, raw in enumerate(text.splitlines()):
        stripped = re.sub(r'"(?:[^"\\]|\\.)*"', '""', raw)
        stripped = re.sub(r"#.*", "", stripped)
        for col, ch in enumerate(stripped):
            if ch == "{": brace_stack.append((lineno, col))
            elif ch == "}":
                if brace_stack: brace_stack.pop()
                else: diags.append(_diag(lineno, col, col+1, "Unexpected '}' — no matching '{'"))
            elif ch == "(": paren_stack.append((lineno, col))
            elif ch == ")":
                if paren_stack: paren_stack.pop()
                else: diags.append(_diag(lineno, col, col+1, "Unexpected ')' — no matching '('"))
        # Unterminated string check
        in_str = False
        for i, ch in enumerate(raw):
            if ch == '"' and (i == 0 or raw[i-1] != '\\'): in_str = not in_str
            if ch == '#' and not in_str: break
        if in_str: diags.append(_diag(lineno, 0, len(raw), "Unterminated string literal"))
    for (ln, col) in brace_stack: diags.append(_diag(ln, col, col+1, "Unmatched '{' — missing '}'"))
    for (ln, col) in paren_stack: diags.append(_diag(ln, col, col+1, "Unmatched '(' — missing ')'"))
    return diags

def _run_diagnostics(uri: str, text: str) -> list[types.Diagnostic]:
    import subprocess, tempfile, os
    binary = _find_prometheus_binary()
    if binary:
        with tempfile.NamedTemporaryFile(suffix=".prm", mode="w", delete=False) as tf:
            tf.write(text); tmp = tf.name
        try:
            result = subprocess.run([binary, tmp], capture_output=True, text=True, timeout=5)
            diags = []
            for line in result.stderr.splitlines():
                m = _LEX_ERROR_RE.match(line) or _PARSE_ERROR_RE.match(line)
                if m:
                    ln = int(m.group(1)) - 1
                    diags.append(_diag(ln, 0, 9999, m.group(2)))
            return diags
        except Exception as e:
            logger.warning("Binary diagnostics failed: %s", e)
        finally:
            os.unlink(tmp)
    return _heuristic_diagnostics(text)

def _publish(ls, uri, text):
    ls.publish_diagnostics(uri, _run_diagnostics(uri, text))

# ---------------------------------------------------------------------------
# Document sync
# ---------------------------------------------------------------------------
@server.feature(types.TEXT_DOCUMENT_DID_OPEN)
def did_open(ls, params: types.DidOpenTextDocumentParams):
    _documents[params.text_document.uri] = params.text_document.text
    _publish(ls, params.text_document.uri, params.text_document.text)

@server.feature(types.TEXT_DOCUMENT_DID_CHANGE)
def did_change(ls, params: types.DidChangeTextDocumentParams):
    uri = params.text_document.uri
    text = params.content_changes[-1].text
    _documents[uri] = text
    _publish(ls, uri, text)

@server.feature(types.TEXT_DOCUMENT_DID_CLOSE)
def did_close(ls, params: types.DidCloseTextDocumentParams):
    _documents.pop(params.text_document.uri, None)

# ---------------------------------------------------------------------------
# Completion
# ---------------------------------------------------------------------------
@server.feature(types.TEXT_DOCUMENT_COMPLETION,
                types.CompletionOptions(trigger_characters=[".", " ", "("]))
def completion(ls, params: types.CompletionParams) -> types.CompletionList:
    uri = params.text_document.uri
    text = _get_text(uri)
    items = []

    if _preceding_dot(text, params.position.line, params.position.character):
        for name, info in LIST_METHODS.items():
            items.append(types.CompletionItem(
                label=name, kind=types.CompletionItemKind.Method,
                detail=info["sig"],
                documentation=types.MarkupContent(
                    kind=types.MarkupKind.Markdown,
                    value=f"**`{info['sig']}`**\n\n{info['doc']}")))
    else:
        for kw in KEYWORDS:
            items.append(types.CompletionItem(
                label=kw, kind=types.CompletionItemKind.Keyword, detail="keyword",
                documentation=types.MarkupContent(kind=types.MarkupKind.Markdown,
                                                  value=KEYWORD_DOCS.get(kw, kw))))
        for t in TYPES:
            items.append(types.CompletionItem(
                label=t, kind=types.CompletionItemKind.Class, detail="type",
                documentation=types.MarkupContent(kind=types.MarkupKind.Markdown,
                                                  value=KEYWORD_DOCS.get(t, t))))
        for name, info in BUILTINS.items():
            items.append(types.CompletionItem(
                label=name, kind=types.CompletionItemKind.Function,
                detail=info["sig"],
                documentation=types.MarkupContent(kind=types.MarkupKind.Markdown,
                                                  value=f"**`{info['sig']}`**\n\n{info['doc']}"),
                insert_text=f"{name}(${{1}})",
                insert_text_format=types.InsertTextFormat.Snippet))
        for mod, doc in STDLIB_MODULES.items():
            items.append(types.CompletionItem(
                label=mod, kind=types.CompletionItemKind.Module, detail="stdlib module",
                documentation=types.MarkupContent(kind=types.MarkupKind.Markdown,
                                                  value=f"**`{mod}`** stdlib module\n\n{doc}")))
        syms = _extract_symbols(text)
        for (name, ln, col, params_raw, ret) in syms["functions"]:
            items.append(types.CompletionItem(
                label=name, kind=types.CompletionItemKind.Function,
                detail=f"func {ret} {name}({params_raw})",
                documentation=types.MarkupContent(kind=types.MarkupKind.Markdown,
                                                  value=f"User-defined function at line {ln+1}.")))
        for (vname, ln, col, vtype) in syms["variables"]:
            items.append(types.CompletionItem(
                label=vname, kind=types.CompletionItemKind.Variable,
                detail=f"{vtype} {vname}",
                documentation=types.MarkupContent(kind=types.MarkupKind.Markdown,
                                                  value=f"Variable declared at line {ln+1}.")))

    return types.CompletionList(is_incomplete=False, items=items)

# ---------------------------------------------------------------------------
# Hover
# ---------------------------------------------------------------------------
@server.feature(types.TEXT_DOCUMENT_HOVER)
def hover(ls, params: types.HoverParams) -> Optional[types.Hover]:
    text = _get_text(params.text_document.uri)
    word = _word_at(text, params.position.line, params.position.character)
    if not word: return None

    def _hover(md: str) -> types.Hover:
        return types.Hover(contents=types.MarkupContent(
            kind=types.MarkupKind.Markdown, value=md))

    if word in BUILTINS:
        i = BUILTINS[word]
        return _hover(f"```prometheus\n{i['sig']}\n```\n\n{i['doc']}")
    if word in LIST_METHODS:
        i = LIST_METHODS[word]
        return _hover(f"```prometheus\n{i['sig']}\n```\n\n{i['doc']}")
    if word in KEYWORD_DOCS:
        return _hover(f"**`{word}`** — {KEYWORD_DOCS[word]}")
    if word in STDLIB_MODULES:
        return _hover(f"**`{word}`** (stdlib module)\n\n{STDLIB_MODULES[word]}")

    syms = _extract_symbols(text)
    for (name, ln, col, params_raw, ret) in syms["functions"]:
        if name == word:
            return _hover(f"```prometheus\nfunc {ret} {name}({params_raw})\n```\n\nDefined at line {ln+1}.")
    for (vname, ln, col, vtype) in syms["variables"]:
        if vname == word:
            return _hover(f"```prometheus\n{vtype} {vname}\n```\n\nDeclared at line {ln+1}.")
    return None

# ---------------------------------------------------------------------------
# Go-to Definition
# ---------------------------------------------------------------------------
@server.feature(types.TEXT_DOCUMENT_DEFINITION)
def definition(ls, params: types.DefinitionParams) -> Optional[types.Location]:
    uri = params.text_document.uri
    text = _get_text(uri)
    word = _word_at(text, params.position.line, params.position.character)
    if not word: return None

    syms = _extract_symbols(text)
    for (name, ln, col, *_) in syms["functions"]:
        if name == word:
            r = types.Range(start=types.Position(line=ln, character=col),
                            end=types.Position(line=ln, character=col+len(name)))
            return types.Location(uri=uri, range=r)
    for (vname, ln, col, _) in syms["variables"]:
        if vname == word:
            r = types.Range(start=types.Position(line=ln, character=col),
                            end=types.Position(line=ln, character=col+len(vname)))
            return types.Location(uri=uri, range=r)
    return None

# ---------------------------------------------------------------------------
# Document Symbols
# ---------------------------------------------------------------------------
@server.feature(types.TEXT_DOCUMENT_DOCUMENT_SYMBOL)
def document_symbol(ls, params: types.DocumentSymbolParams) -> list[types.DocumentSymbol]:
    text = _get_text(params.text_document.uri)
    syms = _extract_symbols(text)
    result = []
    for (name, ln, col, params_raw, ret) in syms["functions"]:
        r = types.Range(start=types.Position(line=ln, character=col),
                        end=types.Position(line=ln, character=col+len(name)))
        result.append(types.DocumentSymbol(name=name, detail=f"func {ret}({params_raw})",
                                           kind=types.SymbolKind.Function,
                                           range=r, selection_range=r))
    for (vname, ln, col, vtype) in syms["variables"]:
        r = types.Range(start=types.Position(line=ln, character=col),
                        end=types.Position(line=ln, character=col+len(vname)))
        result.append(types.DocumentSymbol(name=vname, detail=vtype,
                                           kind=types.SymbolKind.Variable,
                                           range=r, selection_range=r))
    return result

# ---------------------------------------------------------------------------
# Signature Help
# ---------------------------------------------------------------------------
@server.feature(types.TEXT_DOCUMENT_SIGNATURE_HELP,
                types.SignatureHelpOptions(trigger_characters=["(", ","]))
def signature_help(ls, params: types.SignatureHelpParams) -> Optional[types.SignatureHelp]:
    text = _get_text(params.text_document.uri)
    lines = text.splitlines()
    pos = params.position
    if pos.line >= len(lines): return None
    row = lines[pos.line]

    col, depth, arg_index = min(pos.character, len(row)) - 1, 0, 0
    while col >= 0:
        ch = row[col]
        if ch == ")": depth += 1
        elif ch == "(":
            if depth == 0: break
            depth -= 1
        elif ch == "," and depth == 0: arg_index += 1
        col -= 1
    if col < 0: return None

    ne = col
    ns = ne
    while ns > 0 and (row[ns-1].isalnum() or row[ns-1] == "_"): ns -= 1
    func_name = row[ns:ne]
    if not func_name: return None

    if func_name in BUILTINS:
        info = BUILTINS[func_name]
        pl = info["params"]
        return types.SignatureHelp(
            signatures=[types.SignatureInformation(
                label=info["sig"], documentation=info["doc"],
                parameters=[types.ParameterInformation(label=p) for p in pl])],
            active_signature=0, active_parameter=min(arg_index, len(pl)-1))

    syms = _extract_symbols(text)
    for (name, ln, col2, params_raw, ret) in syms["functions"]:
        if name == func_name:
            pl = [p.strip() for p in params_raw.split(",") if p.strip()]
            return types.SignatureHelp(
                signatures=[types.SignatureInformation(
                    label=f"func {ret} {name}({params_raw})",
                    documentation=f"Defined at line {ln+1}.",
                    parameters=[types.ParameterInformation(label=p) for p in pl])],
                active_signature=0, active_parameter=min(arg_index, max(0, len(pl)-1)))
    return None

# ---------------------------------------------------------------------------
# Formatting
# ---------------------------------------------------------------------------
@server.feature(types.TEXT_DOCUMENT_FORMATTING)
def formatting(ls, params: types.DocumentFormattingParams) -> list[types.TextEdit]:
    uri = params.text_document.uri
    text = _get_text(uri)
    opts = params.options
    indent = (" " * (opts.tab_size or 4)) if (opts.insert_spaces if opts.insert_spaces is not None else True) else "\t"

    lines_raw = text.splitlines(keepends=True)
    end_line = len(lines_raw)
    end_char = len(lines_raw[-1]) if lines_raw else 0

    out, level, prev_blank = [], 0, False
    for raw in text.splitlines():
        stripped = raw.strip()
        if not stripped:
            if not prev_blank: out.append("")
            prev_blank = True
            continue
        prev_blank = False
        if stripped.startswith("}"): level = max(0, level - 1)
        out.append(indent * level + stripped)
        if stripped.endswith("{") and not stripped.startswith("#"): level += 1

    formatted = "\n".join(out) + "\n"
    return [types.TextEdit(
        range=types.Range(start=types.Position(line=0, character=0),
                          end=types.Position(line=end_line, character=end_char)),
        new_text=formatted)]

# ---------------------------------------------------------------------------
# Semantic Tokens
# ---------------------------------------------------------------------------
_LEGEND = types.SemanticTokensLegend(
    token_types=["keyword", "type", "function", "variable", "string", "number", "comment", "operator"],
    token_modifiers=[],
)
_TYPE_MAP = {"keyword":0,"type":1,"builtin":2,"ident":3,"string":4,"number":5,"comment":6,"operator":7,"delimiter":7}

@server.feature(types.TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL,
                types.SemanticTokensOptions(legend=_LEGEND, full=True))
def semantic_tokens_full(ls, params: types.SemanticTokensParams) -> types.SemanticTokens:
    text = _get_text(params.text_document.uri)
    tokens = []
    for lineno, row in enumerate(text.splitlines()):
        pos = 0
        while pos < len(row):
            for tname, pat in _TOKEN_PATTERNS:
                m = pat.match(row, pos)
                if m:
                    tokens.append((lineno, m.start(), m.end()-m.start(), _TYPE_MAP.get(tname,3), 0))
                    pos = m.end()
                    break
            else:
                pos += 1

    data, prev_line, prev_col = [], 0, 0
    for (line, col, length, ttype, tmod) in tokens:
        dl = line - prev_line
        dc = col - prev_col if dl == 0 else col
        data.extend([dl, dc, length, ttype, tmod])
        prev_line, prev_col = line, col
    return types.SemanticTokens(data=data)

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
def main():
    import sys
    logger.info("Starting Prometheus Language Server")
    if "--tcp" in sys.argv:
        host, port = "127.0.0.1", 2087
        for i, arg in enumerate(sys.argv):
            if arg == "--host" and i+1 < len(sys.argv): host = sys.argv[i+1]
            if arg == "--port" and i+1 < len(sys.argv): port = int(sys.argv[i+1])
        logger.info("TCP mode on %s:%d", host, port)
        server.start_tcp(host, port)
    else:
        server.start_io()

if __name__ == "__main__":
    main()