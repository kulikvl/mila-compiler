#pragma once
#include <type_traits>
#include <vector>
#include "ASTNodeVisitor.hpp"

/**
 * @brief Visitor that collects all nodes of type T
 */
template <typename T>
class CollectorVisitor : public ASTNodeVisitor {
   public:
    std::vector<T*> collectedNodes;

    template <typename N>
    void tryAddNode(N* node) {
        if constexpr (std::is_same_v<N*, T*>) {
            collectedNodes.push_back(static_cast<T*>(node));
        }
    }

    void visit(PrimitiveTypeASTNode& node) override { tryAddNode(&node); }

    void visit(ArrayTypeASTNode& node) override {
        tryAddNode(&node);
        node.getElemTypeNode()->accept(*this);
    }

    void visit(BinOpASTNode& node) override {
        tryAddNode(&node);
        node.getLhsExprNode()->accept(*this);
        node.getRhsExprNode()->accept(*this);
    }

    void visit(UnaryOpASTNode& node) override {
        tryAddNode(&node);
        node.getExprNode()->accept(*this);
    }

    void visit(LiteralASTNode& node) override { tryAddNode(&node); }

    void visit(DeclVarRefASTNode& node) override { tryAddNode(&node); }

    void visit(DeclArrayRefASTNode& node) override {
        tryAddNode(&node);
        node.getIndexNode()->accept(*this);
    }

    void visit(FunCallASTNode& node) override {
        tryAddNode(&node);
        for (auto& arg : node.getArgNodes())
            arg->accept(*this);
    }

    void visit(BlockASTNode& node) override {
        tryAddNode(&node);
        for (auto& arg : node.getStatementNodes())
            arg->accept(*this);
    }

    void visit(CompoundStmtASTNode& node) override {
        tryAddNode(&node);
        for (auto& arg : node.getStatementNodes())
            arg->accept(*this);
    }

    void visit(VarDeclASTNode& node) override {
        tryAddNode(&node);
        node.getTypeNode()->accept(*this);
    }

    void visit(ArrayDeclASTNode& node) override {
        tryAddNode(&node);
        node.getTypeNode()->accept(*this);
    }

    void visit(ConstDefASTNode& node) override {
        tryAddNode(&node);
        if (node.getTypeNode().has_value())
            node.getTypeNode().value()->accept(*this);
    }

    void visit(ProcDeclASTNode& node) override {
        tryAddNode(&node);
        for (auto& arg : node.getParamNodes())
            arg->accept(*this);
        if (node.getBlockNode().has_value())
            node.getBlockNode().value()->accept(*this);
    }

    void visit(FunDeclASTNode& node) override {
        tryAddNode(&node);
        for (auto& arg : node.getParamNodes())
            arg->accept(*this);
        if (node.getBlockNode().has_value())
            node.getBlockNode().value()->accept(*this);
        node.getRetTypeNode()->accept(*this);
    }

    void visit(AssignASTNode& node) override {
        tryAddNode(&node);
        node.getExprNode()->accept(*this);
        node.getVarNode()->accept(*this);
    }

    void visit(IfASTNode& node) override {
        tryAddNode(&node);
        node.getCondNode()->accept(*this);
        node.getBodyNode()->accept(*this);
        if (node.getElseBodyNode().has_value())
            node.getElseBodyNode().value()->accept(*this);
    }

    void visit(WhileASTNode& node) override {
        tryAddNode(&node);
        node.getBodyNode()->accept(*this);
        node.getCondNode()->accept(*this);
    }

    void visit(ForASTNode& node) override {
        tryAddNode(&node);
        node.getBodyNode()->accept(*this);
        node.getInitNode()->accept(*this);
        node.getToNode()->accept(*this);
    }

    void visit(ProcCallASTNode& node) override {
        tryAddNode(&node);
        for (auto& arg : node.getArgNodes())
            arg->accept(*this);
    }

    void visit(EmptyStmtASTNode& node) override { tryAddNode(&node); }

    void visit(ProgramASTNode& node) override {
        tryAddNode(&node);
        node.getBlockNode()->accept(*this);
    }

    void visit(BreakASTNode& node) override { tryAddNode(&node); }

    void visit(ExitASTNode& node) override { tryAddNode(&node); }
};
