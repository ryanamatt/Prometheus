#ifndef DOT_VISITOR_H
#define DOT_VISITOR_H

#include <string>
#include <ostream>
#include "ast_nodes.h"

/**
 * @brief Walks an AST and emits a Graphviz DOT file to the given stream.
 *
 * Usage:
 *   DOTVisitor visitor(std::cout);          // or an ofstream
 *   visitor.generate(top_level_nodes);
 *
 * Then render with:
 *   dot -Tpng ast.dot -o ast.png
 */
class DOTVisitor {
public:
    explicit DOTVisitor(std::ostream& out) : out_(out) {}

    /**
     * @brief Entry point — emits a complete DOT digraph for all top-level nodes.
     */
    void generate(const std::vector<std::unique_ptr<ASTNode>>& nodes);

private:
    std::ostream& out_;
    int next_id_ = 0;

    /** Allocate a unique integer node ID. */
    int new_id() { return next_id_++; }

    /**
     * @brief Recursively visit a node, emit its DOT declaration, and return
     *        the ID assigned to it so the caller can draw an edge.
     */
    int visit(ASTNode* node);

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------

    /** Emit a labelled node declaration. */
    void node_decl(int id, const std::string& label,
                   const std::string& shape = "box",
                   const std::string& color = "#AEDFF7");

    /** Emit a directed edge between two node IDs. */
    void edge(int parent, int child, const std::string& label = "");

    /** Escape characters that would break a DOT label string. */
    static std::string escape(const std::string& s);
};

#endif // DOT_VISITOR_H