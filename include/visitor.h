#ifndef VISITOR_H
#define VISITOR_H

#include "prometheus_types.h"

// Forward-declare every concrete node type so the visitor interface
// can reference them without pulling in the full ast_nodes.h here.
class NumberNode;
class StringNode;
class BooleanNode;

class VarNode;
class BinOpNode;
class UnaryOpNode;
class VarDeclNode;
class IncrementDecrementNode;

class PrintNode;
class InputNode;
class RangeNode;

class IfNode;
class WhileNode;
class ForNode;
class ForInNode;

class FunctionDeclNode;
class ReturnNode;
class CallNode;

class ListLiteralNode;
class ListDeclNode;
class ListIndexNode;
class ListAssignNode;
class ListAppendNode;
class ListLengthNode;
class ListInsertNode;
class ListPopNode;
class ListRemoveNode;
class ListClearNode;

class ImportNode;
class UseNode;

class EOFNode;

/**
 * @brief Pure-virtual visitor interface for the Prometheus AST.
 *
 * Each concrete node type has a corresponding `visit` overload.
 * Implementors (Interpreter, DOTVisitor, …) derive from this class
 * and provide all overloads.  The dispatch happens via ASTNode::accept(),
 * which calls the right overload through static typing.
 *
 * Return type is PrometheusValue so that the interpreter can use the
 * visitor directly as an expression evaluator.  Non-expression visitors
 * (e.g. DOTVisitor) may simply return std::monostate{} from every overload.
 */
class Visitor {
public:
    virtual ~Visitor() = default;

    virtual PrometheusValue visit(NumberNode* node) = 0;
    virtual PrometheusValue visit(StringNode* node) = 0;
    virtual PrometheusValue visit(BooleanNode* node) = 0;
    
    virtual PrometheusValue visit(VarNode* node) = 0;
    virtual PrometheusValue visit(BinOpNode* node) = 0;
    virtual PrometheusValue visit(UnaryOpNode* node) = 0;
    virtual PrometheusValue visit(VarDeclNode* node) = 0;
    virtual PrometheusValue visit(IncrementDecrementNode* node) = 0;

    virtual PrometheusValue visit(PrintNode* node) = 0;
    virtual PrometheusValue visit(InputNode* node) = 0;
    virtual PrometheusValue visit(RangeNode* node) = 0;

    virtual PrometheusValue visit(IfNode* node) = 0;
    virtual PrometheusValue visit(WhileNode* node) = 0;
    virtual PrometheusValue visit(ForNode* node) = 0;
    virtual PrometheusValue visit(ForInNode* node) = 0;

    virtual PrometheusValue visit(FunctionDeclNode* node) = 0;
    virtual PrometheusValue visit(ReturnNode* node) = 0;
    virtual PrometheusValue visit(CallNode* node) = 0;

    virtual PrometheusValue visit(ListLiteralNode* node) = 0;
    virtual PrometheusValue visit(ListDeclNode* node) = 0;
    virtual PrometheusValue visit(ListIndexNode* node) = 0;
    virtual PrometheusValue visit(ListAssignNode* node) = 0;
    virtual PrometheusValue visit(ListAppendNode* node) = 0;
    virtual PrometheusValue visit(ListLengthNode* node) = 0;
    virtual PrometheusValue visit(ListInsertNode* node) = 0;
    virtual PrometheusValue visit(ListPopNode* node) = 0;
    virtual PrometheusValue visit(ListRemoveNode* node) = 0;
    virtual PrometheusValue visit(ListClearNode* node) = 0;

    virtual PrometheusValue visit(ImportNode* node) = 0;
    virtual PrometheusValue visit(UseNode* node) = 0;

    virtual PrometheusValue visit(EOFNode* node) = 0;
};

#endif // VISITOR_H