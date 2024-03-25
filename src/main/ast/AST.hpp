#pragma once
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <map>
#include <memory>
#include <ostream>
#include <vector>
#include "lexer/Token.hpp"

//------------------------------------------------------------------------------//

// Forward declarations
class ASTNodeVisitor;
class StatementASTNode;
class DeclASTNode;

//------------------------------------------------------------------------------//

/**
 * @brief Base AST node interface
 */
class ASTNode {
   public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTNodeVisitor& visitor) = 0;
};

/**
 * @brief Abstract type node
 */
class TypeASTNode : public ASTNode {
   public:
    /**
     * @brief Type of the types :)
     */
    enum class Type {
        PRIMITIVE,
        ARRAY,
    };

    [[nodiscard]] virtual Type getType() const = 0;

    /**
     * @brief Creates a declaration node with the given identifier and the type of the node that overrides this method
     */
    [[nodiscard]] virtual std::unique_ptr<DeclASTNode> createDeclNode(const std::string& ident) const = 0;
};

/**
 * @brief Simple Type Node: Integer / Real
 */
class PrimitiveTypeASTNode : public TypeASTNode {
   public:
    enum class PrimitiveType {
        INTEGER,
        REAL,
    };

   private:
    PrimitiveType type;

   public:
    explicit PrimitiveTypeASTNode(PrimitiveType type);
    [[nodiscard]] PrimitiveType getPrimitiveType() const;
    [[nodiscard]] TypeASTNode::Type getType() const override;
    [[nodiscard]] std::unique_ptr<DeclASTNode> createDeclNode(const std::string& ident) const override;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Array Type Node
 */
class ArrayTypeASTNode : public TypeASTNode {
   private:
    std::unique_ptr<PrimitiveTypeASTNode> elemTypeNode;
    int lowerBound;
    int upperBound;

   public:
    ArrayTypeASTNode(std::unique_ptr<PrimitiveTypeASTNode> elemTypeNode, int lowerBound, int upperBound);
    [[nodiscard]] const std::unique_ptr<PrimitiveTypeASTNode>& getElemTypeNode() const;
    [[nodiscard]] int getLowerBound() const;
    [[nodiscard]] int getUpperBound() const;
    [[nodiscard]] TypeASTNode::Type getType() const override;
    [[nodiscard]] std::unique_ptr<DeclASTNode> createDeclNode(const std::string& ident) const override;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Expression node interface
 */
class ExprASTNode : public ASTNode {};

/**
 * @brief Binary operation
 */
class BinOpASTNode : public ExprASTNode {
    Token op;
    std::unique_ptr<ExprASTNode> lhsExprNode;
    std::unique_ptr<ExprASTNode> rhsExprNode;

   public:
    BinOpASTNode(Token op, std::unique_ptr<ExprASTNode> lhsExprNode, std::unique_ptr<ExprASTNode> rhsExprNode);
    [[nodiscard]] const Token& getOp() const;
    [[nodiscard]] const std::unique_ptr<ExprASTNode>& getLhsExprNode() const;
    [[nodiscard]] const std::unique_ptr<ExprASTNode>& getRhsExprNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Unary operation
 */
class UnaryOpASTNode : public ExprASTNode {
    Token op;
    std::unique_ptr<ExprASTNode> exprNode;

   public:
    UnaryOpASTNode(Token op, std::unique_ptr<ExprASTNode> exprNode);
    [[nodiscard]] const Token& getOp() const;
    [[nodiscard]] const std::unique_ptr<ExprASTNode>& getExprNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Literal (integer, real, string)
 */
class LiteralASTNode : public ExprASTNode {
    TokenValue value;

   public:
    explicit LiteralASTNode(TokenValue value);
    [[nodiscard]] TokenValue getValue() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Abstract declaration reference for typed symbols (not functions and procedures)
 */
class DeclRefASTNode : public ExprASTNode {
   protected:
    std::string refName;

   public:
    explicit DeclRefASTNode(std::string refName);
    [[nodiscard]] const std::string& getRefName() const;
};

/**
 * @brief Variable/Constant declaration reference
 */
class DeclVarRefASTNode : public DeclRefASTNode {
   public:
    explicit DeclVarRefASTNode(std::string refName);
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Array declaration reference
 */
class DeclArrayRefASTNode : public DeclRefASTNode {
    std::unique_ptr<ExprASTNode> indexNode;

   public:
    DeclArrayRefASTNode(std::string refName, std::unique_ptr<ExprASTNode> indexNode);
    [[nodiscard]] const std::unique_ptr<ExprASTNode>& getIndexNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Function call
 */
class FunCallASTNode : public ExprASTNode {
    std::string funName;
    std::vector<std::unique_ptr<ExprASTNode>> argNodes;

   public:
    FunCallASTNode(std::string funName, std::vector<std::unique_ptr<ExprASTNode>> argNodes);
    [[nodiscard]] const std::string& getFunName() const;
    [[nodiscard]] const std::vector<std::unique_ptr<ExprASTNode>>& getArgNodes() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Statement node interface
 */
class StatementASTNode : public ASTNode {};

/**
 * @brief Block statement
 */
class BlockASTNode : public StatementASTNode {
    /**
     * @brief Is the main block of the program
     * @note In main block, all declared constants/variables are global. And there is only one main block (similar to main() in C).
     */
    bool main = false;
    std::vector<std::unique_ptr<StatementASTNode>> statementNodes;

   public:
    explicit BlockASTNode(std::vector<std::unique_ptr<StatementASTNode>> statementNodes);
    [[nodiscard]] bool isMain() const;
    void setMain(bool _main);
    [[nodiscard]] const std::vector<std::unique_ptr<StatementASTNode>>& getStatementNodes() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Compound statement
 */
class CompoundStmtASTNode : public StatementASTNode {
    std::vector<std::unique_ptr<StatementASTNode>> statementNodes;

   public:
    explicit CompoundStmtASTNode(std::vector<std::unique_ptr<StatementASTNode>> statementNodes);
    [[nodiscard]] const std::vector<std::unique_ptr<StatementASTNode>>& getStatementNodes() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Abstract declaration statement
 */
class DeclASTNode : public StatementASTNode {
    std::string declName;
    bool global = false;

   public:
    explicit DeclASTNode(std::string declName);
    [[nodiscard]] const std::string& getDeclName() const;
    [[nodiscard]] bool isGlobal() const;
    void setGlobal(bool _global);
};

/**
 * @brief Primitive variable declaration
 */
class VarDeclASTNode : public DeclASTNode {
    std::unique_ptr<PrimitiveTypeASTNode> typeNode;

   public:
    VarDeclASTNode(std::string varName, std::unique_ptr<PrimitiveTypeASTNode> typeNode);
    [[nodiscard]] const std::unique_ptr<PrimitiveTypeASTNode>& getTypeNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Array declaration
 */
class ArrayDeclASTNode : public DeclASTNode {
    std::unique_ptr<ArrayTypeASTNode> typeNode;

   public:
    ArrayDeclASTNode(std::string arrayName, std::unique_ptr<ArrayTypeASTNode> typeNode);
    [[nodiscard]] const std::unique_ptr<ArrayTypeASTNode>& getTypeNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Constant definition
 */
class ConstDefASTNode : public DeclASTNode {
    std::unique_ptr<ExprASTNode> exprNode;

    /**
     * @note Type is inferred during codegen from the exprNode
     */
    std::optional<std::unique_ptr<PrimitiveTypeASTNode>> typeNode = std::nullopt;

   public:
    ConstDefASTNode(std::string constName, std::unique_ptr<ExprASTNode> exprNode);
    [[nodiscard]] const std::unique_ptr<ExprASTNode>& getExprNode() const;
    [[nodiscard]] const std::optional<std::unique_ptr<PrimitiveTypeASTNode>>& getTypeNode() const;
    void setTypeNode(std::unique_ptr<PrimitiveTypeASTNode> newTypeNode);
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Procedure declaration
 */
class ProcDeclASTNode : public DeclASTNode {
    std::vector<std::unique_ptr<VarDeclASTNode>> paramNodes;

    /**
     * @note Forward declaration does not have body (definition)
     */
    std::optional<std::unique_ptr<BlockASTNode>> optBlockNode;

   public:
    ProcDeclASTNode(std::string procName, std::vector<std::unique_ptr<VarDeclASTNode>> paramNodes,
                    std::optional<std::unique_ptr<BlockASTNode>> optBlockNode);
    [[nodiscard]] const std::vector<std::unique_ptr<VarDeclASTNode>>& getParamNodes() const;
    [[nodiscard]] const std::optional<std::unique_ptr<BlockASTNode>>& getBlockNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Function declaration
 */
class FunDeclASTNode : public DeclASTNode {
    std::vector<std::unique_ptr<VarDeclASTNode>> paramNodes;
    std::unique_ptr<PrimitiveTypeASTNode> retTypeNode;

    /**
     * @note Forward declaration does not have body (definition)
     */
    std::optional<std::unique_ptr<BlockASTNode>> optBlockNode;

   public:
    FunDeclASTNode(std::string funName, std::vector<std::unique_ptr<VarDeclASTNode>> paramNodes,
                   std::optional<std::unique_ptr<BlockASTNode>> optBlockNode,
                   std::unique_ptr<PrimitiveTypeASTNode> retTypeNode);
    [[nodiscard]] const std::vector<std::unique_ptr<VarDeclASTNode>>& getParamNodes() const;
    [[nodiscard]] const std::unique_ptr<PrimitiveTypeASTNode>& getRetTypeNode() const;
    [[nodiscard]] const std::optional<std::unique_ptr<BlockASTNode>>& getBlockNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Assignment statement
 */
class AssignASTNode : public StatementASTNode {
    std::unique_ptr<DeclRefASTNode> varNode;
    std::unique_ptr<ExprASTNode> exprNode;

   public:
    AssignASTNode(std::unique_ptr<DeclRefASTNode> varNode, std::unique_ptr<ExprASTNode> exprNode);
    [[nodiscard]] const std::unique_ptr<DeclRefASTNode>& getVarNode() const;
    [[nodiscard]] const std::unique_ptr<ExprASTNode>& getExprNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief If statement
 */
class IfASTNode : public StatementASTNode {
    std::unique_ptr<ExprASTNode> condNode;
    std::unique_ptr<StatementASTNode> bodyNode;
    std::optional<std::unique_ptr<StatementASTNode>> optElseBodyNode;

   public:
    IfASTNode(std::unique_ptr<ExprASTNode> condNode, std::unique_ptr<StatementASTNode> bodyNode,
              std::optional<std::unique_ptr<StatementASTNode>> optElseBodyNode);
    [[nodiscard]] const std::unique_ptr<ExprASTNode>& getCondNode() const;
    [[nodiscard]] const std::unique_ptr<StatementASTNode>& getBodyNode() const;
    [[nodiscard]] const std::optional<std::unique_ptr<StatementASTNode>>& getElseBodyNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief While statement
 */
class WhileASTNode : public StatementASTNode {
    std::unique_ptr<ExprASTNode> condNode;
    std::unique_ptr<StatementASTNode> bodyNode;

   public:
    WhileASTNode(std::unique_ptr<ExprASTNode> condNode, std::unique_ptr<StatementASTNode> bodyNode);
    [[nodiscard]] const std::unique_ptr<ExprASTNode>& getCondNode() const;
    [[nodiscard]] const std::unique_ptr<StatementASTNode>& getBodyNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief For statement
 */
class ForASTNode : public StatementASTNode {
    std::unique_ptr<AssignASTNode> initNode;
    std::unique_ptr<ExprASTNode> toNode;
    std::unique_ptr<StatementASTNode> bodyNode;
    bool increasing;

   public:
    ForASTNode(std::unique_ptr<AssignASTNode> initNode, std::unique_ptr<ExprASTNode> toNode,
               std::unique_ptr<StatementASTNode> bodyNode, bool increasing);
    [[nodiscard]] const std::unique_ptr<AssignASTNode>& getInitNode() const;
    [[nodiscard]] const std::unique_ptr<ExprASTNode>& getToNode() const;
    [[nodiscard]] const std::unique_ptr<StatementASTNode>& getBodyNode() const;
    [[nodiscard]] bool isIncreasing() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Procedure call
 */
class ProcCallASTNode : public StatementASTNode {
    std::string procName;
    std::vector<std::unique_ptr<ExprASTNode>> argNodes;

   public:
    ProcCallASTNode(std::string procName, std::vector<std::unique_ptr<ExprASTNode>> argNodes);
    [[nodiscard]] const std::string& getProcName() const;
    [[nodiscard]] const std::vector<std::unique_ptr<ExprASTNode>>& getArgNodes() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Empty statement (like ';')
 */
class EmptyStmtASTNode : public StatementASTNode {
   public:
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Program
 */
class ProgramASTNode : public ASTNode {
    std::string programName;
    std::unique_ptr<BlockASTNode> blockNode;

   public:
    ProgramASTNode(std::string programName, std::unique_ptr<BlockASTNode> blockNode);
    [[nodiscard]] const std::string& getProgramName() const;
    [[nodiscard]] const std::unique_ptr<BlockASTNode>& getBlockNode() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Break instruction (for, while)
 */
class BreakASTNode : public StatementASTNode {
    llvm::BasicBlock* breakBlock = nullptr;

   public:
    void setBreakBlock(llvm::BasicBlock* BB);
    [[nodiscard]] llvm::BasicBlock* getBreakBlock() const;
    void accept(ASTNodeVisitor& visitor) override;
};

/**
 * @brief Exit instruction (procedure, function)
 */
class ExitASTNode : public StatementASTNode {
    //    /**
    //     * @brief Function that loads the return value
    //     * @note nullptr = void return type
    //     */
    //    std::function<llvm::LoadInst*()> loadReturnV = nullptr;
    // todo:

    using RetV = std::variant<std::monostate, std::function<llvm::LoadInst*()>, llvm::Value*>;
    RetV retV = std::monostate{};

   public:
    void setRetV(RetV newRetV);
    [[nodiscard]] const RetV& getRetV() const;
    void accept(ASTNodeVisitor& visitor) override;
};
