#pragma once
#include <experimental/iterator>
#include <iostream>
#include <set>
#include <sstream>
#include "ast/AST.hpp"
#include "lexer/Lexer.hpp"

class Parser {
    /**
     * @brief The lexer used to read tokens
     */
    Lexer lexer;

    Token match(std::initializer_list<TokenType> tokenTypes, const std::string& rule = "[unspecified_rule]");
    Token match(TokenType tokenType, const std::string& rule = "[unspecified_rule]");

    /**
     * @brief Whether to print all grammar rules used during parsing
     */
    bool dumpRules;

    void report(const std::string& rule) const;

   public:
    explicit Parser(Lexer& lexer, bool dumpRules = false);

    /* ----------------- Recursive descent functions ----------------- */

    std::unique_ptr<ProgramASTNode> parseProgram();
    std::unique_ptr<BlockASTNode> parseBlock();
    void parseBlockDecl(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    Token parseUnsignedNumber();
    std::unique_ptr<TypeASTNode> parseType();
    std::unique_ptr<PrimitiveTypeASTNode> parsePrimitiveType();
    std::unique_ptr<ArrayTypeASTNode> parseArrayType();
    int parseSignedInteger();
    void parseConstantDefinitionList(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    void parseConstantDefinitionListR(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    void parseConstantDefinition(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    void parseVariableDeclarationList(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    void parseVariableDeclarationListR(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    void parseVariableDeclarationGroup(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    std::vector<std::string> parseIdentifierList();
    void parseIdentifierListR(std::vector<std::string>& identifiers);
    void parseProcedureDeclaration(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    void parseFunctionDeclaration(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    std::vector<std::unique_ptr<VarDeclASTNode>> parseFunctionParameters();
    std::vector<std::unique_ptr<VarDeclASTNode>> parseFormalParameterList();
    void parseFormalParameterListR(std::vector<std::unique_ptr<VarDeclASTNode>>& parameterNodes);
    void parseParameterGroup(std::vector<std::unique_ptr<VarDeclASTNode>>& parameterNodes);
    std::optional<std::unique_ptr<BlockASTNode>> parseBodyOrForward();
    std::unique_ptr<BlockASTNode> parseBody(); // Body is just a block without function/procedure declarations
    void parseBodyDecl(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    std::unique_ptr<StatementASTNode> parseStatement();
    std::unique_ptr<StatementASTNode> parseSimpleStatement();
    std::unique_ptr<StatementASTNode> parseSimpleStatementIdentifierContinuation(const std::string& identifier);
    std::unique_ptr<DeclArrayRefASTNode> parseOptionalArrayAccess(const std::string& identifier);
    std::unique_ptr<DeclArrayRefASTNode> parseArrayAccess(const std::string& identifier);
    std::unique_ptr<StatementASTNode> parseEmptyStatement();
    std::unique_ptr<StatementASTNode> parseComplexStatement();
    std::unique_ptr<CompoundStmtASTNode> parseCompoundStatement();
    void parseCompoundStatementR(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes);
    std::unique_ptr<StatementASTNode> parseIfStatement();
    std::optional<std::unique_ptr<StatementASTNode>> parseElseStatement();
    std::unique_ptr<WhileASTNode> parseWhileStatement();
    std::unique_ptr<ForASTNode> parseForStatement();
    Token parseTo();
    std::unique_ptr<ExprASTNode> parseExpression();
    std::unique_ptr<ExprASTNode> parseLogicalOrExpression();
    std::unique_ptr<ExprASTNode> parseLogicalOrExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode);
    std::unique_ptr<ExprASTNode> parseLogicalAndExpression();
    std::unique_ptr<ExprASTNode> parseLogicalAndExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode);
    std::unique_ptr<ExprASTNode> parseEqualityExpression();
    std::unique_ptr<ExprASTNode> parseEqualityExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode);
    Token parseEqualityOperator();
    std::unique_ptr<ExprASTNode> parseRelationalExpression();
    std::unique_ptr<ExprASTNode> parseRelationalExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode);
    Token parseRelationalOperator();
    std::unique_ptr<ExprASTNode> parseAdditiveExpression();
    std::unique_ptr<ExprASTNode> parseAdditiveExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode);
    Token parseAdditiveOperator();
    std::unique_ptr<ExprASTNode> parseMultiplicativeExpression();
    std::unique_ptr<ExprASTNode> parseMultiplicativeExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode);
    Token parseMultiplicativeOperator();
    std::unique_ptr<ExprASTNode> parseUnaryExpression();
    Token parseUnaryOperator();
    std::unique_ptr<ExprASTNode> parsePrimaryExpression();
    std::unique_ptr<ExprASTNode> parsePrimaryExpressionIdentifierContinuation(const std::string& identifier);
    std::vector<std::unique_ptr<ExprASTNode>> parseFunctionArgs();
    std::vector<std::unique_ptr<ExprASTNode>> parseArgumentList();
    void parseArgumentListR(std::vector<std::unique_ptr<ExprASTNode>>& argNodes);
};

class ParserException : public std::exception {
   private:
    std::string message;

   public:
    ParserException(const std::string& rule, const Token& actualToken, const std::set<TokenType>& expectedTokenTypes);

    [[nodiscard]] const char* what() const noexcept override;
};
