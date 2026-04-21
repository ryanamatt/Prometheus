#include <sstream>
#include "dot_visitor.h"

// ============================================================================
// Public entry point
// ============================================================================

void DOTVisitor::generate(const std::vector<std::unique_ptr<ASTNode>>& nodes) {
    out_ << "digraph AST {\n";
    out_ << "  graph [fontname=\"Helvetica\" rankdir=TB splines=polyline\n"
            "         nodesep=0.4 ranksep=0.6 ordering=out];\n";
    out_ << "  node  [fontname=\"Helvetica\" style=filled fillcolor=\"#AEDFF7\""
            " shape=box margin=\"0.2,0.1\"];\n";
    out_ << "  edge  [fontname=\"Helvetica\" fontsize=9 arrowsize=0.7];\n\n";

    // Emit an invisible root node so multiple top-level statements are
    // anchored to a common "Program" entry point.
    int root_id = new_id();
    node_decl(root_id, "Program", "ellipse", "#FFD580");

    for (const auto& node : nodes) {
        int child_id = visit(node.get());
        edge(root_id, child_id);
    }

    out_ << "}\n";
}

// ============================================================================
// visit — dispatch on concrete type
// ============================================================================

int DOTVisitor::visit(ASTNode* node) {

    // ------------------------------------------------------------------
    // Literals
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<NumberNode*>(node)) {
        int id = new_id();
        node_decl(id, "Number\\n" + escape(n->value), "ellipse", "#C8E6C9");
        return id;
    }

    if (auto* n = dynamic_cast<StringNode*>(node)) {
        int id = new_id();
        node_decl(id, "String\\n\\\"" + escape(n->value) + "\\\"", "ellipse", "#C8E6C9");
        return id;
    }

    if (auto* n = dynamic_cast<BooleanNode*>(node)) {
        int id = new_id();
        node_decl(id, "Bool\\n" + escape(n->value), "ellipse", "#C8E6C9");
        return id;
    }

    // ------------------------------------------------------------------
    // Variable reference
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<VarNode*>(node)) {
        int id = new_id();
        node_decl(id, "Var\\n" + escape(n->value), "ellipse", "#FFF9C4");
        return id;
    }

    // ------------------------------------------------------------------
    // Binary operation
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<BinOpNode*>(node)) {
        int id = new_id();
        node_decl(id, "BinOp\\n" + escape(n->op.get_value()));
        int left_id  = visit(n->left.get());
        int right_id = visit(n->right.get());
        edge(id, left_id,  "left");
        edge(id, right_id, "right");
        return id;
    }

    // ------------------------------------------------------------------
    // Unary operation
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<UnaryOpNode*>(node)) {
        int id = new_id();
        node_decl(id, "UnaryOp\\n" + escape(n->op.get_value()));
        int operand_id = visit(n->right.get());
        edge(id, operand_id);
        return id;
    }

    // ------------------------------------------------------------------
    // Variable declaration
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<VarDeclNode*>(node)) {
        int id = new_id();
        node_decl(id, "VarDecl\\n" + escape(n->var_type) + " " + escape(n->name),
                  "box", "#AEDFF7");
        int val_id = visit(n->value_node.get());
        edge(id, val_id, "value");
        return id;
    }

    // ------------------------------------------------------------------
    // Print
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<PrintNode*>(node)) {
        int id = new_id();
        node_decl(id, "Print", "box", "#E1BEE7");
        for (size_t i = 0; i < n->expressions.size(); i++) {
            int expr_id = visit(n->expressions[i].get());
            edge(id, expr_id, "arg" + std::to_string(i));
        }
        return id;
    }

    // ------------------------------------------------------------------
    // Input
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<InputNode*>(node)) {
        int id = new_id();
        node_decl(id, "Input\\n\\\"" + escape(n->msg) + "\\\"", "box", "#E1BEE7");
        return id;
    }

    // ------------------------------------------------------------------
    // If / elif / else
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<IfNode*>(node)) {
        int id = new_id();
        node_decl(id, "If", "diamond", "#FFCCBC");

        int cond_id = visit(n->condition.get());
        edge(id, cond_id, "cond");

        // then branch — group under a cluster node
        int then_id = new_id();
        node_decl(then_id, "Then", "box", "#FFE0B2");
        edge(id, then_id);
        for (auto& stmt : n->then_branch) {
            int s_id = visit(stmt.get());
            edge(then_id, s_id);
        }

        // elif branches
        for (size_t i = 0; i < n->elif_branches.size(); i++) {
            int elif_id = new_id();
            node_decl(elif_id, "Elif " + std::to_string(i + 1), "diamond", "#FFCCBC");
            edge(id, elif_id);
            int elif_cond = visit(n->elif_branches[i].first.get());
            edge(elif_id, elif_cond, "cond");
            for (auto& stmt : n->elif_branches[i].second) {
                int s_id = visit(stmt.get());
                edge(elif_id, s_id);
            }
        }

        // else branch
        if (!n->else_branch.empty()) {
            int else_id = new_id();
            node_decl(else_id, "Else", "box", "#FFE0B2");
            edge(id, else_id);
            for (auto& stmt : n->else_branch) {
                int s_id = visit(stmt.get());
                edge(else_id, s_id);
            }
        }

        return id;
    }

    // ------------------------------------------------------------------
    // While
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<WhileNode*>(node)) {
        int id = new_id();
        node_decl(id, "While", "diamond", "#FFCCBC");
        int cond_id = visit(n->condition.get());
        edge(id, cond_id, "cond");
        for (auto& stmt : n->do_branch) {
            int s_id = visit(stmt.get());
            edge(id, s_id, "body");
        }
        return id;
    }

    // ------------------------------------------------------------------
    // For
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<ForNode*>(node)) {
        int id = new_id();
        node_decl(id, "For", "diamond", "#FFCCBC");
        int init_id = visit(n->variable.get());
        int cond_id = visit(n->condition.get());
        int step_id = visit(n->change_var.get());
        edge(id, init_id, "init");
        edge(id, cond_id, "cond");
        edge(id, step_id, "step");
        for (auto& stmt : n->do_branch) {
            int s_id = visit(stmt.get());
            edge(id, s_id, "body");
        }
        return id;
    }

    // ------------------------------------------------------------------
    // Function declaration
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<FunctionDeclNode*>(node)) {
        int id = new_id();

        // Build a label that lists all parameters
        std::string params_str;
        for (size_t i = 0; i < n->params.size(); i++) {
            if (i) params_str += ", ";
            params_str += n->params[i].first + " " + n->params[i].second;
        }
        node_decl(id, "FuncDecl\\n" + escape(n->return_type) + " " +
                      escape(n->name) + "(" + escape(params_str) + ")",
                  "box", "#B3E5FC");

        for (auto& stmt : n->body) {
            int s_id = visit(stmt.get());
            edge(id, s_id);
        }
        return id;
    }

    // ------------------------------------------------------------------
    // Return
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<ReturnNode*>(node)) {
        int id = new_id();
        node_decl(id, "Return", "box", "#B3E5FC");
        int val_id = visit(n->value_node.get());
        edge(id, val_id, "value");
        return id;
    }

    // ------------------------------------------------------------------
    // Function call
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<CallNode*>(node)) {
        int id = new_id();
        node_decl(id, "Call\\n" + escape(n->name) + "()", "box", "#D1C4E9");
        for (size_t i = 0; i < n->args.size(); i++) {
            int arg_id = visit(n->args[i].get());
            edge(id, arg_id, "arg" + std::to_string(i));
        }
        return id;
    }

    // ------------------------------------------------------------------
    // Increment / Decrement
    // ------------------------------------------------------------------

    if (auto* n = dynamic_cast<IncrementDecrementNode*>(node)) {
        int id = new_id();
        std::string op = (n->inc_val > 0) ? "++" : "--";
        node_decl(id, escape(op) + "\\n" + escape(n->name), "ellipse", "#FFF9C4");
        return id;
    }

    // ------------------------------------------------------------------
    // EOF sentinel
    // ------------------------------------------------------------------

    if (dynamic_cast<EOFNode*>(node)) {
        int id = new_id();
        node_decl(id, "EOF", "ellipse", "#ECEFF1");
        return id;
    }

    // Fallback for any unknown node type
    int id = new_id();
    node_decl(id, "Unknown", "box", "#EF9A9A");
    return id;
}

// ============================================================================
// Helpers
// ============================================================================

void DOTVisitor::node_decl(int id, const std::string& label,
                            const std::string& shape,
                            const std::string& color) {
    out_ << "  n" << id
         << " [label=\"" << label << "\""
         << ", shape=" << shape
         << ", fillcolor=\"" << color << "\""
         << "];\n";
}

void DOTVisitor::edge(int parent, int child, const std::string& label) {
    out_ << "  n" << parent << " -> n" << child;
    if (!label.empty())
        out_ << " [label=\"" << label << "\"]";
    out_ << ";\n";
}

std::string DOTVisitor::escape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '<':  out += "\\<";  break;
            case '>':  out += "\\>";  break;
            case '{':  out += "\\{";  break;
            case '}':  out += "\\}";  break;
            case '|':  out += "\\|";  break;
            default:   out += c;
        }
    }
    return out;
}