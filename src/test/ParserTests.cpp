#include <gtest/gtest.h>
#include <sstream>
#include "parser/Parser.hpp"

// It is much more easier and efficient to test codegen, than to test parser alone

TEST(ParserTests, HandlesEmptyProgram) {
    std::istringstream input("program test ; begin end .");
    Lexer lexer(input);
    Parser parser(lexer);

    auto programNode = parser.parseProgram();
    ASSERT_EQ("test", programNode->getProgramName());
}

TEST(ParserTests, HandlesPrimitiveType) {
    std::istringstream input("integer");
    Lexer lexer(input);
    Parser parser(lexer);

    auto typeNode = parser.parseType();
    ASSERT_EQ(TypeASTNode::Type::PRIMITIVE, typeNode->getType());

    auto primitiveTypeNode = dynamic_cast<PrimitiveTypeASTNode*>(typeNode.get());
    ASSERT_EQ(PrimitiveTypeASTNode::PrimitiveType::INTEGER, primitiveTypeNode->getPrimitiveType());
}

TEST(ParserTests, HandlesProcedureDeclaration) {
    std::istringstream input("{...} procedure proc() ; forward ; begin {...}");
    Lexer lexer(input);
    Parser parser(lexer);

    std::vector<std::unique_ptr<StatementASTNode>> statementNodes;
    parser.parseProcedureDeclaration(statementNodes);

    ASSERT_EQ(1, statementNodes.size());

    auto procDeclNode = dynamic_cast<ProcDeclASTNode*>(statementNodes[0].get());

    ASSERT_EQ("proc", procDeclNode->getDeclName());
    ASSERT_EQ(0, procDeclNode->getParamNodes().size());
    ASSERT_EQ(std::nullopt, procDeclNode->getBlockNode());
}

TEST(ParserTests, HandlesAdditiveOperator) {
    std::istringstream input("+");
    Lexer lexer(input);
    Parser parser(lexer);

    auto token = parser.parseAdditiveOperator();
    ASSERT_TRUE(token.getType() == TokenType::PLUS);
}

TEST(ParserTests, HandlesMultiplicativeOperator) {
    std::istringstream input("*");
    Lexer lexer(input);
    Parser parser(lexer);

    auto token = parser.parseMultiplicativeOperator();
    ASSERT_TRUE(token.getType() == TokenType::MULTIPLY);
}
