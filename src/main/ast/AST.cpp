#include "AST.hpp"
#include <utility>
#include "visitor/ASTNodeVisitor.hpp"

PrimitiveTypeASTNode::PrimitiveTypeASTNode(PrimitiveType type) : type(type) {}

PrimitiveTypeASTNode::PrimitiveType PrimitiveTypeASTNode::getPrimitiveType() const {
    return type;
}

TypeASTNode::Type PrimitiveTypeASTNode::getType() const {
    return TypeASTNode::Type::PRIMITIVE;
}

std::unique_ptr<DeclASTNode> PrimitiveTypeASTNode::createDeclNode(const std::string& ident) const {
    return std::make_unique<VarDeclASTNode>(ident, std::make_unique<PrimitiveTypeASTNode>(type));
}

void PrimitiveTypeASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

ArrayTypeASTNode::ArrayTypeASTNode(std::unique_ptr<PrimitiveTypeASTNode> elemTypeNode, int lowerBound, int upperBound)
    : elemTypeNode(std::move(elemTypeNode)), lowerBound(lowerBound), upperBound(upperBound) {}

const std::unique_ptr<PrimitiveTypeASTNode>& ArrayTypeASTNode::getElemTypeNode() const {
    return elemTypeNode;
}

int ArrayTypeASTNode::getLowerBound() const {
    return lowerBound;
}

int ArrayTypeASTNode::getUpperBound() const {
    return upperBound;
}

TypeASTNode::Type ArrayTypeASTNode::getType() const {
    return TypeASTNode::Type::ARRAY;
}

std::unique_ptr<DeclASTNode> ArrayTypeASTNode::createDeclNode(const std::string& ident) const {
    return std::make_unique<ArrayDeclASTNode>(
        ident, std::make_unique<ArrayTypeASTNode>(
                   std::make_unique<PrimitiveTypeASTNode>(elemTypeNode->getPrimitiveType()), lowerBound, upperBound));
}

void ArrayTypeASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

BinOpASTNode::BinOpASTNode(Token op, std::unique_ptr<ExprASTNode> lhsExprNode, std::unique_ptr<ExprASTNode> rhsExprNode)
    : op(std::move(op)), lhsExprNode(std::move(lhsExprNode)), rhsExprNode(std::move(rhsExprNode)) {}

const Token& BinOpASTNode::getOp() const {
    return op;
}

const std::unique_ptr<ExprASTNode>& BinOpASTNode::getLhsExprNode() const {
    return lhsExprNode;
}

const std::unique_ptr<ExprASTNode>& BinOpASTNode::getRhsExprNode() const {
    return rhsExprNode;
}

void BinOpASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

UnaryOpASTNode::UnaryOpASTNode(Token op, std::unique_ptr<ExprASTNode> exprNode)
    : op(std::move(op)), exprNode(std::move(exprNode)) {}

const Token& UnaryOpASTNode::getOp() const {
    return op;
}

const std::unique_ptr<ExprASTNode>& UnaryOpASTNode::getExprNode() const {
    return exprNode;
}

void UnaryOpASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

LiteralASTNode::LiteralASTNode(TokenValue value) : value(std::move(value)) {}

TokenValue LiteralASTNode::getValue() const {
    return value;
}

void LiteralASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

DeclRefASTNode::DeclRefASTNode(std::string refName) : refName(std::move(refName)) {}

const std::string& DeclRefASTNode::getRefName() const {
    return refName;
}

DeclVarRefASTNode::DeclVarRefASTNode(std::string refName) : DeclRefASTNode(std::move(refName)) {}

void DeclVarRefASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

DeclArrayRefASTNode::DeclArrayRefASTNode(std::string refName, std::unique_ptr<ExprASTNode> indexNode)
    : DeclRefASTNode(std::move(refName)), indexNode(std::move(indexNode)) {}

const std::unique_ptr<ExprASTNode>& DeclArrayRefASTNode::getIndexNode() const {
    return indexNode;
}

void DeclArrayRefASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

FunCallASTNode::FunCallASTNode(std::string funName, std::vector<std::unique_ptr<ExprASTNode>> argNodes)
    : funName(std::move(funName)), argNodes(std::move(argNodes)) {}

const std::string& FunCallASTNode::getFunName() const {
    return funName;
}

const std::vector<std::unique_ptr<ExprASTNode>>& FunCallASTNode::getArgNodes() const {
    return argNodes;
}

void FunCallASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

BlockASTNode::BlockASTNode(std::vector<std::unique_ptr<StatementASTNode>> statementNodes)
    : statementNodes(std::move(statementNodes)) {}

const std::vector<std::unique_ptr<StatementASTNode>>& BlockASTNode::getStatementNodes() const {
    return statementNodes;
}

bool BlockASTNode::isMain() const {
    return main;
}

void BlockASTNode::setMain(bool _main) {
    main = _main;
}

void BlockASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

CompoundStmtASTNode::CompoundStmtASTNode(std::vector<std::unique_ptr<StatementASTNode>> statementNodes)
    : statementNodes(std::move(statementNodes)) {}

const std::vector<std::unique_ptr<StatementASTNode>>& CompoundStmtASTNode::getStatementNodes() const {
    return statementNodes;
}

void CompoundStmtASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

DeclASTNode::DeclASTNode(std::string declName) : declName(std::move(declName)) {}

bool DeclASTNode::isGlobal() const {
    return global;
}

void DeclASTNode::setGlobal(bool _global) {
    global = _global;
}

const std::string& DeclASTNode::getDeclName() const {
    return declName;
}

VarDeclASTNode::VarDeclASTNode(std::string varName, std::unique_ptr<PrimitiveTypeASTNode> typeNode)
    : DeclASTNode(std::move(varName)), typeNode(std::move(typeNode)) {}

const std::unique_ptr<PrimitiveTypeASTNode>& VarDeclASTNode::getTypeNode() const {
    return typeNode;
}

void VarDeclASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

ArrayDeclASTNode::ArrayDeclASTNode(std::string arrayName, std::unique_ptr<ArrayTypeASTNode> typeNode)
    : DeclASTNode(std::move(arrayName)), typeNode(std::move(typeNode)) {}

const std::unique_ptr<ArrayTypeASTNode>& ArrayDeclASTNode::getTypeNode() const {
    return typeNode;
}

void ArrayDeclASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

ConstDefASTNode::ConstDefASTNode(std::string constName, std::unique_ptr<ExprASTNode> exprNode)
    : DeclASTNode(std::move(constName)), exprNode(std::move(exprNode)) {}

const std::unique_ptr<ExprASTNode>& ConstDefASTNode::getExprNode() const {
    return exprNode;
}

const std::optional<std::unique_ptr<PrimitiveTypeASTNode>>& ConstDefASTNode::getTypeNode() const {
    return typeNode;
}

void ConstDefASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

void ConstDefASTNode::setTypeNode(std::unique_ptr<PrimitiveTypeASTNode> newTypeNode) {
    typeNode = std::move(newTypeNode);
}

ProcDeclASTNode::ProcDeclASTNode(std::string procName, std::vector<std::unique_ptr<VarDeclASTNode>> paramNodes,
                                 std::optional<std::unique_ptr<BlockASTNode>> optBlockNode)
    : DeclASTNode(std::move(procName)), paramNodes(std::move(paramNodes)), optBlockNode(std::move(optBlockNode)) {}

const std::vector<std::unique_ptr<VarDeclASTNode>>& ProcDeclASTNode::getParamNodes() const {
    return paramNodes;
}

const std::optional<std::unique_ptr<BlockASTNode>>& ProcDeclASTNode::getBlockNode() const {
    return optBlockNode;
}

void ProcDeclASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

FunDeclASTNode::FunDeclASTNode(std::string funName, std::vector<std::unique_ptr<VarDeclASTNode>> paramNodes,
                               std::optional<std::unique_ptr<BlockASTNode>> optBlockNode,
                               std::unique_ptr<PrimitiveTypeASTNode> retTypeNode)
    : DeclASTNode(std::move(funName)),
      paramNodes(std::move(paramNodes)),
      retTypeNode(std::move(retTypeNode)),
      optBlockNode(std::move(optBlockNode)) {}

const std::vector<std::unique_ptr<VarDeclASTNode>>& FunDeclASTNode::getParamNodes() const {
    return paramNodes;
}

const std::unique_ptr<PrimitiveTypeASTNode>& FunDeclASTNode::getRetTypeNode() const {
    return retTypeNode;
}

const std::optional<std::unique_ptr<BlockASTNode>>& FunDeclASTNode::getBlockNode() const {
    return optBlockNode;
}

void FunDeclASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

AssignASTNode::AssignASTNode(std::unique_ptr<DeclRefASTNode> varNode, std::unique_ptr<ExprASTNode> exprNode)
    : varNode(std::move(varNode)), exprNode(std::move(exprNode)) {}

const std::unique_ptr<DeclRefASTNode>& AssignASTNode::getVarNode() const {
    return varNode;
}

const std::unique_ptr<ExprASTNode>& AssignASTNode::getExprNode() const {
    return exprNode;
}

void AssignASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

IfASTNode::IfASTNode(std::unique_ptr<ExprASTNode> condNode, std::unique_ptr<StatementASTNode> bodyNode,
                     std::optional<std::unique_ptr<StatementASTNode>> optElseBodyNode)
    : condNode(std::move(condNode)), bodyNode(std::move(bodyNode)), optElseBodyNode(std::move(optElseBodyNode)) {}

const std::unique_ptr<ExprASTNode>& IfASTNode::getCondNode() const {
    return condNode;
}

const std::unique_ptr<StatementASTNode>& IfASTNode::getBodyNode() const {
    return bodyNode;
}

const std::optional<std::unique_ptr<StatementASTNode>>& IfASTNode::getElseBodyNode() const {
    return optElseBodyNode;
}

void IfASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

WhileASTNode::WhileASTNode(std::unique_ptr<ExprASTNode> condNode, std::unique_ptr<StatementASTNode> bodyNode)
    : condNode(std::move(condNode)), bodyNode(std::move(bodyNode)) {}

const std::unique_ptr<ExprASTNode>& WhileASTNode::getCondNode() const {
    return condNode;
}

const std::unique_ptr<StatementASTNode>& WhileASTNode::getBodyNode() const {
    return bodyNode;
}

void WhileASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

ForASTNode::ForASTNode(std::unique_ptr<AssignASTNode> initNode, std::unique_ptr<ExprASTNode> toNode,
                       std::unique_ptr<StatementASTNode> bodyNode, bool increasing)
    : initNode(std::move(initNode)), toNode(std::move(toNode)), bodyNode(std::move(bodyNode)), increasing(increasing) {}

const std::unique_ptr<AssignASTNode>& ForASTNode::getInitNode() const {
    return initNode;
}

const std::unique_ptr<ExprASTNode>& ForASTNode::getToNode() const {
    return toNode;
}

const std::unique_ptr<StatementASTNode>& ForASTNode::getBodyNode() const {
    return bodyNode;
}

bool ForASTNode::isIncreasing() const {
    return increasing;
}

void ForASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

ProcCallASTNode::ProcCallASTNode(std::string procName, std::vector<std::unique_ptr<ExprASTNode>> argNodes)
    : procName(std::move(procName)), argNodes(std::move(argNodes)) {}

const std::string& ProcCallASTNode::getProcName() const {
    return procName;
}

const std::vector<std::unique_ptr<ExprASTNode>>& ProcCallASTNode::getArgNodes() const {
    return argNodes;
}

void ProcCallASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

void EmptyStmtASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

ProgramASTNode::ProgramASTNode(std::string programName, std::unique_ptr<BlockASTNode> blockNode)
    : programName(std::move(programName)), blockNode(std::move(blockNode)) {}

const std::string& ProgramASTNode::getProgramName() const {
    return programName;
}

const std::unique_ptr<BlockASTNode>& ProgramASTNode::getBlockNode() const {
    return blockNode;
}

void ProgramASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

void BreakASTNode::setBreakBlock(llvm::BasicBlock* BB) {
    breakBlock = BB;
}

llvm::BasicBlock* BreakASTNode::getBreakBlock() const {
    return breakBlock;
}

void BreakASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}

void ExitASTNode::setRetV(ExitASTNode::RetV newRetV) {
    retV = std::move(newRetV);
}

const ExitASTNode::RetV& ExitASTNode::getRetV() const {
    return retV;
}

void ExitASTNode::accept(ASTNodeVisitor& visitor) {
    visitor.visit(*this);
}
