import os
import re
from pathlib import Path

def parse_prm_file(file_path):
    """Parses a .prm file for function signatures and doc-comments."""
    functions = []
    current_docs = []
    
    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            # Capture comments as documentation
            if line.startswith("#"):
                comment = line.lstrip("# ").strip()
                if comment:
                    current_docs.append(comment)
            # Match function signature: func type name(args)
            elif line.startswith("func"):
                match = re.match(r"func\s+(\w+)\s+(\w+)\s*\((.*?)\)", line)
                if match:
                    ret_type, name, args = match.groups()
                    functions.append({
                        "name": name,
                        "return_type": ret_type,
                        "args": args,
                        "description": " ".join(current_docs)
                    })
                current_docs = [] # Reset for next function
            else:
                if not line: # Empty lines reset doc context
                    current_docs = []
    return functions

def generate_markdown():
    source_dir = Path("stdlib")
    output_dir = Path("docs/stdlib")
    output_dir.mkdir(parents=True, exist_ok=True)

    for prm_file in source_dir.glob("*.prm"):
        module_name = prm_file.stem.capitalize()
        funcs = parse_prm_file(prm_file)
        
        md_content = [f"# {module_name} Module Reference\n"]
        md_content.append(f"Documentation automatically generated from `{prm_file.name}`.\n")
        
        for f in funcs:
            md_content.append(f"## `{f['name']}`")
            md_content.append(f"**Signature:** `func {f['return_type']} {f['name']}({f['args']})`  ")
            if f['description']:
                md_content.append(f"\n{f['description']}\n")
            md_content.append("---\n")

        output_path = output_dir / f"{prm_file.stem}.md"
        with open(output_path, "w") as f:
            f.write("\n".join(md_content))
        print(f"Generated: {output_path}")

if __name__ == "__main__":
    generate_markdown()