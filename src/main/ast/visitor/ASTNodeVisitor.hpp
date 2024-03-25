#pragma once
#include "ast/AST.hpp"

/**
 * @brief Abstract Visitor for AST nodes
 */
class ASTNodeVisitor {
   public:
    virtual ~ASTNodeVisitor() = default;

    virtual void visit(PrimitiveTypeASTNode& node) = 0;
    virtual void visit(ArrayTypeASTNode& node) = 0;
    virtual void visit(BinOpASTNode& node) = 0;
    virtual void visit(UnaryOpASTNode& node) = 0;
    virtual void visit(LiteralASTNode& node) = 0;
    virtual void visit(DeclVarRefASTNode& node) = 0;
    virtual void visit(DeclArrayRefASTNode& node) = 0;
    virtual void visit(FunCallASTNode& node) = 0;
    virtual void visit(BlockASTNode& node) = 0;
    virtual void visit(CompoundStmtASTNode& node) = 0;
    virtual void visit(VarDeclASTNode& node) = 0;
    virtual void visit(ArrayDeclASTNode& node) = 0;
    virtual void visit(ConstDefASTNode& node) = 0;
    virtual void visit(ProcDeclASTNode& node) = 0;
    virtual void visit(FunDeclASTNode& node) = 0;
    virtual void visit(AssignASTNode& node) = 0;
    virtual void visit(IfASTNode& node) = 0;
    virtual void visit(WhileASTNode& node) = 0;
    virtual void visit(ForASTNode& node) = 0;
    virtual void visit(ProcCallASTNode& node) = 0;
    virtual void visit(EmptyStmtASTNode& node) = 0;
    virtual void visit(ProgramASTNode& node) = 0;
    virtual void visit(BreakASTNode& node) = 0;
    virtual void visit(ExitASTNode& node) = 0;
};
