#pragma once
#include "ASTNodeVisitor.hpp"

/**
 * @brief Default visitor implementation
 * @note Useful for visitors that only a few nodes use
 */
class DefaultVisitor : public ASTNodeVisitor {
   private:
    bool throws;

   public:
    explicit DefaultVisitor(bool throws = true);

    void visit() const;

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
