# Prometheus Language Server (prometheus-lsp)

A full [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) implementation for the **Prometheus** (`.prm`) scripting language.

## Features

| Feature | Details |
|---|---|
| **Diagnostics** | Real-time error squiggles. Uses the `prometheus` binary when available, falls back to heuristic bracket/string analysis. |
| **Completion** | Keywords, types, builtins (`print`, `input`, `range`, `type`), stdlib modules (`math`, `random`, `time`), list methods, user-defined functions & variables. |
| **Hover** | Inline documentation for all builtins, types, keywords, and user symbols. |
| **Go-to Definition** | Jump to function/variable declarations within the current file. |
| **Document Symbols** | Outline panel listing all functions and top-level variables. |
| **Signature Help** | Parameter hints for builtins and user functions while you type `(` or `,`. |
| **Formatting** | Brace-aware auto-indent (`Format Document` / `Shift+Alt+F`). |
| **Semantic Tokens** | Fine-grained token classification for richer editor themes. |

---

## Installation

### Python server

```bash
pip install -e .
```

This installs the `prometheus-lsp` command on your `PATH`.

### VS Code extension

1. Open `vscode-extension/` in VS Code.
2. Run `npm install`.
3. Press **F5** to launch the Extension Development Host, **or** package it:

```bash
npm install -g @vscode/vsce
npx @vscode/vsce package --allow-missing-repository
code --install-extension prometheus-language-support-0.1.1-dev.vsix
```

## Configuration (VS Code)

| Setting | Default | Description |
|---|---|---|
| `prometheusLsp.serverPath` | `"prometheus-lsp"` | Path or command name of the LSP server executable. |
| `prometheusLsp.binaryPath` | `""` | Path to the `prometheus` interpreter for accurate diagnostics. |
| `prometheusLsp.trace.server` | `"off"` | LSP protocol trace level (`"off"` / `"messages"` / `"verbose"`). |

---

---

## Architecture

```
prometheus-lsp/
├── prometheus_lsp/
│   ├── __init__.py
│   └── server.py          ← all LSP handlers (pygls-based)
├── vscode-extension/
│   ├── package.json
│   ├── extension.js        ← VS Code LanguageClient bootstrap
│   ├── language-configuration.json   ← brackets, comments, indent rules
│   └── syntaxes/
│       └── prometheus.tmGrammar.json ← TextMate grammar
└── README.md
```

The server is pure Python built on [pygls](https://github.com/openlawlibrary/pygls). All LSP handlers live in `server.py`; the heavy lifting is done by two subsystems:

* **Symbol extractor** – regex-based scan of the open document to find `func` declarations and typed variable declarations, powering completion, hover, go-to-def, and the outline.
* **Diagnostic engine** – spawns the `prometheus` binary against a temp file and parses its stderr; falls back to heuristic bracket/string checking when the binary is not on `PATH`.
