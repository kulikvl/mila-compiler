#pragma once
#include "ASTNodeVisitor.hpp"
#include <sstream>

/**
 * @brief Visitor for printing AST nodes
 */
class PrintVisitor : public ASTNodeVisitor {
   private:
    std::ostream &os;
    int indent;

   public:
    explicit PrintVisitor(std::ostream &os, int indent = 0);

    void visit(PrimitiveTypeASTNode& node) override;
    void visit(ArrayTypeASTNode& node) override;
    void visit(BinOpASTNode& node) override;
    void visit(UnaryOpASTNode& node) override;
    void visit(LiteralASTNode& node) override;
    void visit(DeclVarRefASTNode& node) override;
    void visit(DeclArrayRefASTNode& node) override;
    void visit(FunCallASTNode& node) override;
    void visit(BlockASTNode& node) override;
    void visit(CompoundStmtASTNode& node) override;
    void visit(VarDeclASTNode& node) override;
    void visit(ArrayDeclASTNode& node) override;
    void visit(ConstDefASTNode& node) override;
    void visit(ProcDeclASTNode& node) override;
    void visit(FunDeclASTNode& node) override;
    void visit(AssignASTNode& node) override;
    void visit(IfASTNode& node) override;
    void visit(WhileASTNode& node) override;
    void visit(ForASTNode& node) override;
    void visit(ProcCallASTNode& node) override;
    void visit(EmptyStmtASTNode& node) override;
    void visit(ProgramASTNode& node) override;
    void visit(BreakASTNode& node) override;
    void visit(ExitASTNode& node) override;
};
