#include "Parser.hpp"

Parser::Parser(Lexer& lexer, bool dumpRules) : lexer(lexer), dumpRules(dumpRules) {}

void Parser::report(const std::string& rule) const {
    if (!dumpRules)
        return;
    std::cout << rule << std::endl;
}

Token Parser::match(std::initializer_list<TokenType> tokenTypes, const std::string& rule) {
    for (const auto& tokenType : tokenTypes) {
        if (auto token = lexer.match(tokenType)) {
            std::ostringstream oss;
            oss << "match " << token.value();
            report(oss.str());

            return token.value();
        }
    }

    throw ParserException(rule, lexer.peek(), tokenTypes);
}

Token Parser::match(TokenType tokenType, const std::string& rule) {
    return match({tokenType}, rule);
}

/* ----------------- Recursive descent functions ----------------- */

std::unique_ptr<ProgramASTNode> Parser::parseProgram() {
    switch (lexer.peek().getType()) {
        case TokenType::PROGRAM: {
            report("Program -> <PROGRAM> <IDENTIFIER> <SEMICOLON> Block <DOT>");
            match(TokenType::PROGRAM, "Program");
            auto identToken = match(TokenType::IDENTIFIER, "Program");
            match(TokenType::SEMICOLON, "Program");
            auto blockNode = parseBlock();
            match(TokenType::DOT, "Program");
            return std::make_unique<ProgramASTNode>(std::get<std::string>(identToken.getValue().value()),
                                                    std::move(blockNode));
        }
        default:
            throw ParserException("Program", lexer.peek(), {TokenType::PROGRAM});
    }
}

std::unique_ptr<BlockASTNode> Parser::parseBlock() {
    switch (lexer.peek().getType()) {
        case TokenType::CONST:
        case TokenType::VAR:
        case TokenType::PROCEDURE:
        case TokenType::FUNCTION:
        case TokenType::BEGIN: {
            report("Block -> BlockDecl CompoundStatement");
            std::vector<std::unique_ptr<StatementASTNode>> statementNodes;
            parseBlockDecl(statementNodes);
            statementNodes.push_back(parseCompoundStatement());
            return std::make_unique<BlockASTNode>(std::move(statementNodes));
        }
        default:
            throw ParserException(
                "Block", lexer.peek(),
                {TokenType::CONST, TokenType::VAR, TokenType::PROCEDURE, TokenType::FUNCTION, TokenType::BEGIN});
    }
}

void Parser::parseBlockDecl(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::CONST: {
            report("BlockDecl ⟶ ConstantDefinitionList BlockDecl");
            parseConstantDefinitionList(statementNodes);
            parseBlockDecl(statementNodes);
            break;
        }
        case TokenType::VAR: {
            report("BlockDecl ⟶ VariableDeclarationList BlockDecl");
            parseVariableDeclarationList(statementNodes);
            parseBlockDecl(statementNodes);
            break;
        }
        case TokenType::PROCEDURE: {
            report("BlockDecl ⟶ ProcedureDeclaration BlockDecl");
            parseProcedureDeclaration(statementNodes);
            parseBlockDecl(statementNodes);
            break;
        }
        case TokenType::FUNCTION: {
            report("BlockDecl ⟶ FunctionDeclaration BlockDecl");
            parseFunctionDeclaration(statementNodes);
            parseBlockDecl(statementNodes);
            break;
        }
        case TokenType::BEGIN: {
            report("BlockDecl ->");
            break;
        }
        default:
            throw ParserException(
                "BlockDecl", lexer.peek(),
                {TokenType::CONST, TokenType::VAR, TokenType::PROCEDURE, TokenType::FUNCTION, TokenType::BEGIN});
    }
}

Token Parser::parseUnsignedNumber() {
    switch (lexer.peek().getType()) {
        case TokenType::INTEGER_LITERAL: {
            report("UnsignedNumber -> <INTEGER_LITERAL>");
            return match(TokenType::INTEGER_LITERAL, "UnsignedNumber");
        }
        case TokenType::REAL_LITERAL: {
            report("UnsignedNumber -> <REAL_LITERAL>");
            return match(TokenType::REAL_LITERAL, "UnsignedNumber");
        }
        default:
            throw ParserException("UnsignedNumber", lexer.peek(),
                                  {TokenType::REAL_LITERAL, TokenType::INTEGER_LITERAL});
    }
}

std::unique_ptr<TypeASTNode> Parser::parseType() {
    switch (lexer.peek().getType()) {
        case TokenType::INTEGER:
        case TokenType::REAL: {
            report("Type -> PrimitiveType");
            return parsePrimitiveType();
        }
        case TokenType::ARRAY: {
            report("Type -> ArrayType");
            return parseArrayType();
        }
        default:
            throw ParserException("Type", lexer.peek(), {TokenType::REAL, TokenType::INTEGER, TokenType::ARRAY});
    }
}

std::unique_ptr<PrimitiveTypeASTNode> Parser::parsePrimitiveType() {
    switch (lexer.peek().getType()) {
        case TokenType::REAL: {
            report("PrimitiveType -> <REAL>");
            match(TokenType::REAL, "PrimitiveType");
            return std::make_unique<PrimitiveTypeASTNode>(PrimitiveTypeASTNode::PrimitiveType::REAL);
        }
        case TokenType::INTEGER: {
            report("PrimitiveType -> <INTEGER>");
            match(TokenType::INTEGER, "PrimitiveType");
            return std::make_unique<PrimitiveTypeASTNode>(PrimitiveTypeASTNode::PrimitiveType::INTEGER);
        }
        default:
            throw ParserException("PrimitiveType", lexer.peek(), {TokenType::REAL, TokenType::INTEGER});
    }
}

std::unique_ptr<ArrayTypeASTNode> Parser::parseArrayType() {
    switch (lexer.peek().getType()) {
        case TokenType::ARRAY: {
            report(
                "ArrayType -> <ARRAY> <LEFT_BRACKET> SignedInteger <DOUBLE_DOT> SignedInteger <RIGHT_BRACKET> <OF> "
                "SimpleType");
            match(TokenType::ARRAY, "ArrayType");
            match(TokenType::LEFT_BRACKET, "ArrayType");
            auto lowerBound = parseSignedInteger();
            match(TokenType::DOUBLE_DOT, "ArrayType");
            auto upperBound = parseSignedInteger();
            match(TokenType::RIGHT_BRACKET, "ArrayType");
            match(TokenType::OF, "ArrayType");
            auto typeNode = parsePrimitiveType();
            return std::make_unique<ArrayTypeASTNode>(std::move(typeNode), lowerBound, upperBound);
        }
        default:
            throw ParserException("ArrayType", lexer.peek(), {TokenType::ARRAY});
    }
}

int Parser::parseSignedInteger() {
    switch (lexer.peek().getType()) {
        case TokenType::INTEGER_LITERAL: {
            report("SignedInteger -> <INTEGER_LITERAL>");
            auto intToken = match(TokenType::INTEGER_LITERAL, "SignedInteger");
            int value = std::get<int>(intToken.getValue().value());
            return value;
        }
        case TokenType::MINUS: {
            report("SignedInteger -> <MINUS> <INTEGER_LITERAL>");
            match(TokenType::MINUS, "SignedInteger");
            auto intToken = match(TokenType::INTEGER_LITERAL, "SignedInteger");
            int value = std::get<int>(intToken.getValue().value());
            return -value;
        }
        default:
            throw ParserException("SignedInteger", lexer.peek(), {TokenType::INTEGER_LITERAL, TokenType::MINUS});
    }
}

void Parser::parseConstantDefinitionList(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::CONST: {
            report("ConstantDefinitionList -> <CONST> ConstantDefinition ConstantDefinitionListR");
            match(TokenType::CONST, "ConstantDefinitionList");
            parseConstantDefinition(statementNodes);
            parseConstantDefinitionListR(statementNodes);
            break;
        }
        default:
            throw ParserException("ConstantDefinitionList", lexer.peek(), {TokenType::CONST});
    }
}

void Parser::parseConstantDefinitionListR(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::IDENTIFIER: {
            report("ConstantDefinitionListR -> ConstantDefinition ConstantDefinitionListR");
            parseConstantDefinition(statementNodes);
            parseConstantDefinitionListR(statementNodes);
            break;
        }
        case TokenType::CONST:
        case TokenType::VAR:
        case TokenType::PROCEDURE:
        case TokenType::FUNCTION:
        case TokenType::BEGIN: {
            report("ConstantDefinitionListR ->");
            break;
        }
        default:
            throw ParserException("ConstantDefinitionListR", lexer.peek(),
                                  {TokenType::IDENTIFIER, TokenType::CONST, TokenType::VAR, TokenType::PROCEDURE,
                                   TokenType::FUNCTION, TokenType::BEGIN});
    }
}

void Parser::parseConstantDefinition(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::IDENTIFIER: {
            report("ConstantDefinition -> <IDENTIFIER> <EQUAL> Expression <SEMICOLON>");
            auto identToken = match(TokenType::IDENTIFIER, "ConstantDefinition");
            match(TokenType::EQUAL, "ConstantDefinition");
            auto exprNode = parseExpression();
            match(TokenType::SEMICOLON, "ConstantDefinition");
            auto constDefNode = std::make_unique<ConstDefASTNode>(std::get<std::string>(identToken.getValue().value()),
                                                                  std::move(exprNode));
            statementNodes.push_back(std::move(constDefNode));
            break;
        }
        default:
            throw ParserException("ConstantDefinition", lexer.peek(), {TokenType::IDENTIFIER});
    }
}

void Parser::parseVariableDeclarationList(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::VAR: {
            report("VariableDeclarationList -> <VAR> VariableDeclarationGroup VariableDeclarationListR");
            match(TokenType::VAR, "VariableDeclarationList");
            parseVariableDeclarationGroup(statementNodes);
            parseVariableDeclarationListR(statementNodes);
            break;
        }
        default:
            throw ParserException("VariableDeclarationList", lexer.peek(), {TokenType::VAR});
    }
}

void Parser::parseVariableDeclarationListR(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::IDENTIFIER: {
            report("VariableDeclarationListR -> VariableDeclarationGroup VariableDeclarationListR");
            parseVariableDeclarationGroup(statementNodes);
            parseVariableDeclarationListR(statementNodes);
            break;
        }
        case TokenType::CONST:
        case TokenType::VAR:
        case TokenType::PROCEDURE:
        case TokenType::FUNCTION:
        case TokenType::BEGIN: {
            report("VariableDeclarationListR ->");
            break;
        }
        default:
            throw ParserException("VariableDeclarationListR", lexer.peek(),
                                  {TokenType::IDENTIFIER, TokenType::CONST, TokenType::VAR, TokenType::PROCEDURE,
                                   TokenType::FUNCTION, TokenType::BEGIN});
    }
}

void Parser::parseVariableDeclarationGroup(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::IDENTIFIER: {
            report("VariableDeclarationGroup -> IdentifierList <COLON> Type <SEMICOLON>");
            auto idents = parseIdentifierList();
            match(TokenType::COLON, "VariableDeclarationGroup");
            auto commonTypeNode = parseType();
            for (const auto& ident : idents)
                statementNodes.push_back(commonTypeNode->createDeclNode(ident));
            match(TokenType::SEMICOLON, "VariableDeclarationGroup");
            break;
        }
        default:
            throw ParserException("VariableDeclarationGroup", lexer.peek(), {TokenType::IDENTIFIER});
    }
}

std::vector<std::string> Parser::parseIdentifierList() {
    switch (lexer.peek().getType()) {
        case TokenType::IDENTIFIER: {
            report("IdentifierList -> <IDENTIFIER> IdentifierListR");
            auto identToken = match(TokenType::IDENTIFIER, "IdentifierList");
            std::vector<std::string> idents;
            idents.push_back(std::get<std::string>(identToken.getValue().value()));
            parseIdentifierListR(idents);
            return idents;
        }
        default:
            throw ParserException("IdentifierList", lexer.peek(), {TokenType::IDENTIFIER});
    }
}

void Parser::parseIdentifierListR(std::vector<std::string>& identifiers) {
    switch (lexer.peek().getType()) {
        case TokenType::COMMA: {
            report("IdentifierListR -> <COMMA> <IDENTIFIER> IdentifierListR");
            match(TokenType::COMMA, "IdentifierListR");
            auto identToken = match(TokenType::IDENTIFIER, "IdentifierListR");
            identifiers.push_back(std::get<std::string>(identToken.getValue().value()));
            parseIdentifierListR(identifiers);
            break;
        }
        case TokenType::COLON: {
            report("IdentifierListR ->");
            break;
        }
        default:
            throw ParserException("IdentifierListR", lexer.peek(), {TokenType::COMMA, TokenType::COLON});
    }
}

void Parser::parseProcedureDeclaration(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::PROCEDURE: {
            report(
                "ProcedureDeclaration -> <PROCEDURE> <IDENTIFIER> FunctionParameters <SEMICOLON> "
                "BodyOrForward <SEMICOLON>");
            match(TokenType::PROCEDURE, "ProcedureDeclaration");
            auto identToken = match(TokenType::IDENTIFIER, "ProcedureDeclaration");
            auto paramNodes = parseFunctionParameters();
            match(TokenType::SEMICOLON, "ProcedureDeclaration");
            auto optBlockNode = parseBodyOrForward();
            statementNodes.push_back(std::make_unique<ProcDeclASTNode>(
                std::get<std::string>(identToken.getValue().value()), std::move(paramNodes), std::move(optBlockNode)));
            match(TokenType::SEMICOLON, "ProcedureDeclaration");
            break;
        }
        default:
            throw ParserException("ProcedureDeclaration", lexer.peek(), {TokenType::PROCEDURE});
    }
}

void Parser::parseFunctionDeclaration(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::FUNCTION: {
            report(
                "FunctionDeclaration -> <FUNCTION> <IDENTIFIER> FunctionParameters <COLON> PrimitiveType "
                "<SEMICOLON> "
                "BodyOrForward <SEMICOLON>");
            match(TokenType::FUNCTION, "FunctionDeclaration");
            auto identToken = match(TokenType::IDENTIFIER, "FunctionDeclaration");
            auto paramNodes = parseFunctionParameters();
            match(TokenType::COLON, "FunctionDeclaration");
            auto retPrimitiveTypeNode = parsePrimitiveType();
            match(TokenType::SEMICOLON, "FunctionDeclaration");
            auto optBlockNode = parseBodyOrForward();
            statementNodes.push_back(std::make_unique<FunDeclASTNode>(
                std::get<std::string>(identToken.getValue().value()), std::move(paramNodes), std::move(optBlockNode),
                std::move(retPrimitiveTypeNode)));
            match(TokenType::SEMICOLON, "FunctionDeclaration");
            break;
        }
        default:
            throw ParserException("FunctionDeclaration", lexer.peek(), {TokenType::FUNCTION});
    }
}

std::vector<std::unique_ptr<VarDeclASTNode>> Parser::parseFunctionParameters() {
    switch (lexer.peek().getType()) {
        case TokenType::LEFT_PAREN: {
            report("FunctionParameters -> <LEFT_PAREN> FormalParameterList <RIGHT_PAREN>");
            match(TokenType::LEFT_PAREN, "FunctionDeclaration");
            auto paramNodes = parseFormalParameterList();
            match(TokenType::RIGHT_PAREN, "FunctionDeclaration");
            return paramNodes;
        }
        default:
            throw ParserException("FunctionParameters", lexer.peek(), {TokenType::LEFT_PAREN});
    }
}

std::vector<std::unique_ptr<VarDeclASTNode>> Parser::parseFormalParameterList() {
    switch (lexer.peek().getType()) {
        case TokenType::IDENTIFIER: {
            report("FormalParameterList -> ParameterGroup FormalParameterListR");
            std::vector<std::unique_ptr<VarDeclASTNode>> paramNodes;
            parseParameterGroup(paramNodes);
            parseFormalParameterListR(paramNodes);
            return paramNodes;
        }
        case TokenType::RIGHT_PAREN: {
            report("FormalParameterList ->");
            return {};
        }
        default:
            throw ParserException("FormalParameterList", lexer.peek(), {TokenType::IDENTIFIER, TokenType::RIGHT_PAREN});
    }
}

void Parser::parseFormalParameterListR(std::vector<std::unique_ptr<VarDeclASTNode>>& parameterNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::SEMICOLON: {
            report("FormalParameterListR -> <SEMICOLON> ParameterGroup FormalParameterListR");
            match(TokenType::SEMICOLON, "FormalParameterListR");
            parseParameterGroup(parameterNodes);
            parseFormalParameterListR(parameterNodes);
            break;
        }
        case TokenType::RIGHT_PAREN: {
            report("FormalParameterListR ->");
            break;
        }
        default:
            throw ParserException("FormalParameterListR", lexer.peek(), {TokenType::SEMICOLON, TokenType::RIGHT_PAREN});
    }
}

void Parser::parseParameterGroup(std::vector<std::unique_ptr<VarDeclASTNode>>& parameterNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::IDENTIFIER: {
            report("ParameterGroup -> IdentifierList <COLON> PrimitiveType");
            auto idents = parseIdentifierList();
            match(TokenType::COLON, "ParameterGroup");
            auto commonTypeNode = parsePrimitiveType();
            for (const auto& ident : idents) {
                auto typeNode = std::make_unique<PrimitiveTypeASTNode>(commonTypeNode->getPrimitiveType());
                parameterNodes.push_back(std::make_unique<VarDeclASTNode>(ident, std::move(typeNode)));
            }
            break;
        }
        default:
            throw ParserException("ParameterGroup", lexer.peek(), {TokenType::IDENTIFIER});
    }
}

std::optional<std::unique_ptr<BlockASTNode>> Parser::parseBodyOrForward() {
    switch (lexer.peek().getType()) {
        case TokenType::FORWARD: {
            report("BodyOrForward -> <FORWARD>");
            match(TokenType::FORWARD, "BodyOrForward");
            return std::nullopt;
        }
        case TokenType::BEGIN:
        case TokenType::CONST:
        case TokenType::VAR: {
            report("BodyOrForward -> Body");
            return parseBody();
        }
        default:
            throw ParserException("BodyOrForward", lexer.peek(),
                                  {TokenType::FORWARD, TokenType::BEGIN, TokenType::CONST, TokenType::VAR});
    }
}

std::unique_ptr<BlockASTNode> Parser::parseBody() {
    switch (lexer.peek().getType()) {
        case TokenType::CONST:
        case TokenType::VAR:
        case TokenType::BEGIN: {
            report("Body -> BodyDecl CompoundStatement");
            std::vector<std::unique_ptr<StatementASTNode>> statementNodes;
            parseBodyDecl(statementNodes);
            statementNodes.push_back(parseCompoundStatement());
            return std::make_unique<BlockASTNode>(std::move(statementNodes));
        }
        default:
            throw ParserException("Body", lexer.peek(), {TokenType::BEGIN, TokenType::CONST, TokenType::VAR});
    }

}

void Parser::parseBodyDecl(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::CONST: {
            report("BodyDecl -> ConstantDefinitionList BodyDecl");
            parseConstantDefinitionList(statementNodes);
            parseBodyDecl(statementNodes);
            break;
        }
        case TokenType::VAR: {
            report("BodyDecl -> VariableDeclarationList BodyDecl");
            parseVariableDeclarationList(statementNodes);
            parseBodyDecl(statementNodes);
            break;
        }
        case TokenType::BEGIN: {
            report("BodyDecl ->");
            break;
        }
        default:
            throw ParserException("BodyDecl", lexer.peek(), {TokenType::CONST, TokenType::VAR, TokenType::BEGIN});
    }
}

std::unique_ptr<StatementASTNode> Parser::parseStatement() {
    switch (lexer.peek().getType()) {
        case TokenType::EXIT:
        case TokenType::BREAK:
        case TokenType::IDENTIFIER:
        case TokenType::SEMICOLON:
        case TokenType::END:
        case TokenType::ELSE: {
            report("Statement -> SimpleStatement");
            return parseSimpleStatement();
        }
        case TokenType::BEGIN:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::FOR: {
            report("Statement -> ComplexStatement");
            return parseComplexStatement();
        }
        default:
            throw ParserException(
                "Statement", lexer.peek(),
                {TokenType::ELSE, TokenType::BREAK, TokenType::SEMICOLON, TokenType::EXIT, TokenType::IDENTIFIER,
                 TokenType::BEGIN, TokenType::IF, TokenType::FOR, TokenType::WHILE, TokenType::END});
    }
}

std::unique_ptr<StatementASTNode> Parser::parseSimpleStatement() {
    switch (lexer.peek().getType()) {
        case TokenType::ELSE:
        case TokenType::END:
        case TokenType::SEMICOLON: {
            report("SimpleStatement -> EmptyStatement");
            return parseEmptyStatement();
        }
        case TokenType::EXIT: {
            report("SimpleStatement -> <EXIT>");
            match(TokenType::EXIT, "SimpleStatement");
            return std::make_unique<ExitASTNode>();
        }
        case TokenType::BREAK: {
            report("SimpleStatement -> <BREAK>");
            match(TokenType::BREAK, "SimpleStatement");
            return std::make_unique<BreakASTNode>();
        }
        case TokenType::IDENTIFIER: {
            report("SimpleStatement -> <IDENTIFIER> SimpleStatementIdentifierContinuation");
            auto identifierToken = match(TokenType::IDENTIFIER, "SimpleStatement");
            return parseSimpleStatementIdentifierContinuation(
                std::get<std::string>(identifierToken.getValue().value()));
        }
        default:
            throw ParserException("SimpleStatement", lexer.peek(),
                                  {TokenType::ELSE, TokenType::END, TokenType::BREAK, TokenType::SEMICOLON,
                                   TokenType::EXIT, TokenType::IDENTIFIER});
    }
}

std::unique_ptr<StatementASTNode> Parser::parseSimpleStatementIdentifierContinuation(const std::string& identifier) {
    switch (lexer.peek().getType()) {
        case TokenType::LEFT_PAREN: {
            report("SimpleStatementIdentifierContinuation -> FunctionArgs");
            return std::make_unique<ProcCallASTNode>(identifier, parseFunctionArgs());
        }
        case TokenType::LEFT_BRACKET:
        case TokenType::ASSIGN: {
            report("SimpleStatementIdentifierContinuation -> OptionalArrayAccess <ASSIGN> Expression");
            auto arrayRefNode = parseOptionalArrayAccess(identifier);
            match(TokenType::ASSIGN, "SimpleStatementIdentifierContinuation");
            if (arrayRefNode)
                return std::make_unique<AssignASTNode>(std::move(arrayRefNode), parseExpression());
            else
                return std::make_unique<AssignASTNode>(std::make_unique<DeclVarRefASTNode>(identifier),
                                                       parseExpression());
        }
        default:
            throw ParserException("SimpleStatementIdentifierContinuation", lexer.peek(),
                                  {TokenType::ASSIGN, TokenType::LEFT_BRACKET, TokenType::LEFT_PAREN});
    }
}

std::unique_ptr<DeclArrayRefASTNode> Parser::parseOptionalArrayAccess(const std::string& identifier) {
    switch (lexer.peek().getType()) {
        case TokenType::LEFT_BRACKET: {
            report("OptionalArrayAccess -> ArrayAccess");
            return parseArrayAccess(identifier);
        }
        case TokenType::ASSIGN: {
            report("OptionalArrayAccess ->");
            return nullptr;
        }
        default:
            throw ParserException("ArrayAccess", lexer.peek(), {TokenType::LEFT_BRACKET, TokenType::ASSIGN});
    }
}

std::unique_ptr<DeclArrayRefASTNode> Parser::parseArrayAccess(const std::string& identifier) {
    switch (lexer.peek().getType()) {
        case TokenType::LEFT_BRACKET: {
            report("ArrayAccess -> <LEFT_BRACKET> Expression <RIGHT_BRACKET>");
            match(TokenType::LEFT_BRACKET, "ArrayAccess");
            auto indexNode = parseExpression();
            match(TokenType::RIGHT_BRACKET, "ArrayAccess");
            return std::make_unique<DeclArrayRefASTNode>(identifier, std::move(indexNode));
        }
        default:
            throw ParserException("ArrayAccess", lexer.peek(), {TokenType::LEFT_BRACKET});
    }
}

std::unique_ptr<StatementASTNode> Parser::parseEmptyStatement() {
    switch (lexer.peek().getType()) {
        case TokenType::ELSE:
        case TokenType::END:
        case TokenType::SEMICOLON: {
            report("EmptyStatement ->");
            return std::make_unique<EmptyStmtASTNode>();
        }
        default:
            throw ParserException("EmptyStatement", lexer.peek(),
                                  {TokenType::ELSE, TokenType::SEMICOLON, TokenType::END});
    }
}

std::unique_ptr<StatementASTNode> Parser::parseComplexStatement() {
    switch (lexer.peek().getType()) {
        case TokenType::BEGIN: {
            report("ComplexStatement -> CompoundStatement");
            return parseCompoundStatement();
        }
        case TokenType::IF: {
            report("ComplexStatement -> IfStatement");
            return parseIfStatement();
        }
        case TokenType::WHILE: {
            report("ComplexStatement -> WhileStatement");
            return parseWhileStatement();
        }
        case TokenType::FOR: {
            report("ComplexStatement -> ForStatement");
            return parseForStatement();
        }
        default:
            throw ParserException("ComplexStatement", lexer.peek(),
                                  {TokenType::BEGIN, TokenType::IF, TokenType::FOR, TokenType::WHILE});
    }
}

std::unique_ptr<CompoundStmtASTNode> Parser::parseCompoundStatement() {
    switch (lexer.peek().getType()) {
        case TokenType::BEGIN: {
            report("CompoundStatement -> <BEGIN> Statement CompoundStatementR <END>");
            std::vector<std::unique_ptr<StatementASTNode>> statementNodes;
            match(TokenType::BEGIN, "CompoundStatement");
            statementNodes.push_back(parseStatement());
            parseCompoundStatementR(statementNodes);
            match(TokenType::END, "CompoundStatement");
            return std::make_unique<CompoundStmtASTNode>(std::move(statementNodes));
        }
        default:
            throw ParserException("CompoundStatement", lexer.peek(), {TokenType::BEGIN});
    }
}

void Parser::parseCompoundStatementR(std::vector<std::unique_ptr<StatementASTNode>>& statementNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::SEMICOLON: {
            report("CompoundStatementR -> <SEMICOLON> Statement CompoundStatementR");
            match(TokenType::SEMICOLON, "CompoundStatementR");
            statementNodes.push_back(parseStatement());
            parseCompoundStatementR(statementNodes);
            break;
        }
        case TokenType::END: {
            report("CompoundStatementR ->");
            break;
        }
        default:
            throw ParserException("CompoundStatementR", lexer.peek(), {TokenType::SEMICOLON, TokenType::END});
    }
}

std::unique_ptr<StatementASTNode> Parser::parseIfStatement() {
    switch (lexer.peek().getType()) {
        case TokenType::IF: {
            report("IfStatement -> <IF> Expression <THEN> Statement ElseStatement");
            match(TokenType::IF, "IfStatement");
            auto condNode = parseExpression();
            match(TokenType::THEN, "IfStatement");
            auto bodyNode = parseStatement();
            auto elseBodyNode = parseElseStatement();
            return std::make_unique<IfASTNode>(std::move(condNode), std::move(bodyNode), std::move(elseBodyNode));
        }
        default:
            throw ParserException("IfStatement", lexer.peek(), {TokenType::IF});
    }
}

// !!! Else statement is always connected with the deepest if statement to solve ambiguity
std::optional<std::unique_ptr<StatementASTNode>> Parser::parseElseStatement() {
    switch (lexer.peek().getType()) {
        case TokenType::ELSE: {
            report("ElseStatement-> <ELSE> Statement");
            match(TokenType::ELSE, "ElseStatement");
            return parseStatement();
        }
        case TokenType::END:
        case TokenType::SEMICOLON: {
            report("ElseStatement ->");
            return std::nullopt;
        }
        default:
            throw ParserException("ElseStatement", lexer.peek(),
                                  {TokenType::ELSE, TokenType::END, TokenType::SEMICOLON});
    }
}

std::unique_ptr<WhileASTNode> Parser::parseWhileStatement() {
    switch (lexer.peek().getType()) {
        case TokenType::WHILE: {
            report("WhileStatement -> <WHILE> Expression <DO> Statement");
            match(TokenType::WHILE, "WhileStatement");
            auto condNode = parseExpression();
            match(TokenType::DO, "WhileStatement");
            auto bodyNode = parseStatement();
            return std::make_unique<WhileASTNode>(std::move(condNode), std::move(bodyNode));
        }
        default:
            throw ParserException("WhileStatement", lexer.peek(), {TokenType::WHILE});
    }
}

std::unique_ptr<ForASTNode> Parser::parseForStatement() {
    switch (lexer.peek().getType()) {
        case TokenType::FOR: {
            report("ForStatement -> <FOR> <IDENTIFIER> <ASSIGN> Expression <TO> Expression <DO> Statement");
            match(TokenType::FOR, "ForStatement");
            auto identToken = match(TokenType::IDENTIFIER, "ForStatement");
            match(TokenType::ASSIGN, "ForStatement");
            auto initNode = std::make_unique<AssignASTNode>(
                std::make_unique<DeclVarRefASTNode>(std::get<std::string>(identToken.getValue().value())),
                parseExpression());
            auto toToken = parseTo();
            bool increasing = toToken.getType() == TokenType::TO;
            auto toNode = parseExpression();
            match(TokenType::DO, "ForStatement");
            auto statementNode = parseStatement();
            return std::make_unique<ForASTNode>(std::move(initNode), std::move(toNode), std::move(statementNode),
                                                increasing);
        }
        default:
            throw ParserException("ForStatement", lexer.peek(), {TokenType::FOR});
    }
}

Token Parser::parseTo() {
    switch (lexer.peek().getType()) {
        case TokenType::TO: {
            report("To -> <TO>");
            return match(TokenType::TO, "To");
        }
        case TokenType::DOWNTO: {
            report("To -> <DOWNTO>");
            return match(TokenType::DOWNTO, "To");
        }
        default:
            throw ParserException("To", lexer.peek(), {TokenType::TO, TokenType::DOWNTO});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseExpression() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::IDENTIFIER:
        case TokenType::LEFT_PAREN:
        case TokenType::INTEGER_LITERAL:
        case TokenType::REAL_LITERAL: {
            report("Expression -> LogicalOrExpression");
            return parseLogicalOrExpression();
        }
        default:
            throw ParserException("Expression", lexer.peek(),
                                  {TokenType::MINUS, TokenType::NOT, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
                                   TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseLogicalOrExpression() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::IDENTIFIER:
        case TokenType::LEFT_PAREN:
        case TokenType::INTEGER_LITERAL:
        case TokenType::REAL_LITERAL: {
            report("LogicalOrExpression -> LogicalAndExpression LogicalOrExpressionR");
            auto lhsExprNode = parseLogicalAndExpression();
            return parseLogicalOrExpressionR(std::move(lhsExprNode));
        }
        default:
            throw ParserException("LogicalOrExpression", lexer.peek(),
                                  {TokenType::MINUS, TokenType::NOT, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
                                   TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseLogicalOrExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode) {
    switch (lexer.peek().getType()) {
        case TokenType::OR: {
            report("LogicalOrExpressionR -> <OR> LogicalAndExpression LogicalOrExpressionR");
            auto op = match(TokenType::OR, "LogicalOrExpressionR");
            auto rhsExprNode = parseLogicalAndExpression();
            auto newLhsExprNode = std::make_unique<BinOpASTNode>(op, std::move(lhsExprNode), std::move(rhsExprNode));
            return parseLogicalOrExpressionR(std::move(newLhsExprNode));
        }
        case TokenType::SEMICOLON:
        case TokenType::RIGHT_BRACKET:
        case TokenType::THEN:
        case TokenType::DO:
        case TokenType::TO:
        case TokenType::DOWNTO:
        case TokenType::RIGHT_PAREN:
        case TokenType::COMMA:
        case TokenType::END:
        case TokenType::ELSE: {
            report("LogicalOrExpressionR ->");
            return lhsExprNode;
        }
        default:
            throw ParserException("LogicalOrExpressionR", lexer.peek(),
                                  {TokenType::OR, TokenType::RIGHT_BRACKET, TokenType::THEN, TokenType::DO,
                                   TokenType::TO, TokenType::DOWNTO, TokenType::RIGHT_PAREN, TokenType::COMMA,
                                   TokenType::SEMICOLON, TokenType::END, TokenType::ELSE});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseLogicalAndExpression() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::IDENTIFIER:
        case TokenType::LEFT_PAREN:
        case TokenType::INTEGER_LITERAL:
        case TokenType::REAL_LITERAL: {
            report("LogicalAndExpression -> EqualityExpression LogicalAndExpressionR");
            auto lhsExprNode = parseEqualityExpression();
            return parseLogicalAndExpressionR(std::move(lhsExprNode));
        }
        default:
            throw ParserException("LogicalAndExpression", lexer.peek(),
                                  {TokenType::MINUS, TokenType::NOT, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
                                   TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseLogicalAndExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode) {
    switch (lexer.peek().getType()) {
        case TokenType::AND: {
            report("LogicalAndExpressionR -> <AND> EqualityExpression LogicalAndExpressionR");
            auto op = match(TokenType::AND, "LogicalAndExpressionR");
            auto rhsExprNode = parseEqualityExpression();
            auto newLhsExprNode = std::make_unique<BinOpASTNode>(op, std::move(lhsExprNode), std::move(rhsExprNode));
            return parseLogicalAndExpressionR(std::move(newLhsExprNode));
        }
        case TokenType::OR:
        case TokenType::SEMICOLON:
        case TokenType::RIGHT_BRACKET:
        case TokenType::THEN:
        case TokenType::DO:
        case TokenType::TO:
        case TokenType::DOWNTO:
        case TokenType::RIGHT_PAREN:
        case TokenType::COMMA:
        case TokenType::END:
        case TokenType::ELSE: {
            report("LogicalAndExpressionR ->");
            return lhsExprNode;
        }
        default:
            throw ParserException("LogicalAndExpressionR", lexer.peek(),
                                  {TokenType::AND, TokenType::OR, TokenType::RIGHT_BRACKET, TokenType::THEN,
                                   TokenType::DO, TokenType::TO, TokenType::DOWNTO, TokenType::RIGHT_PAREN,
                                   TokenType::COMMA, TokenType::SEMICOLON, TokenType::END, TokenType::ELSE});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseEqualityExpression() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::IDENTIFIER:
        case TokenType::LEFT_PAREN:
        case TokenType::INTEGER_LITERAL:
        case TokenType::REAL_LITERAL: {
            report("EqualityExpression -> RelationalExpression EqualityExpressionR");
            auto lhsExprNode = parseRelationalExpression();
            return parseEqualityExpressionR(std::move(lhsExprNode));
        }
        default:
            throw ParserException("EqualityExpression", lexer.peek(),
                                  {TokenType::MINUS, TokenType::NOT, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
                                   TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseEqualityExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode) {
    switch (lexer.peek().getType()) {
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL: {
            report("EqualityExpressionR -> EqualityOperator RelationalExpression EqualityExpressionR");
            auto op = parseEqualityOperator();
            auto rhsExprNode = parseRelationalExpression();
            auto newLhsExprNode = std::make_unique<BinOpASTNode>(op, std::move(lhsExprNode), std::move(rhsExprNode));
            return parseEqualityExpressionR(std::move(newLhsExprNode));
        }
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::SEMICOLON:
        case TokenType::RIGHT_BRACKET:
        case TokenType::THEN:
        case TokenType::DO:
        case TokenType::TO:
        case TokenType::DOWNTO:
        case TokenType::RIGHT_PAREN:
        case TokenType::COMMA:
        case TokenType::END:
        case TokenType::ELSE: {
            report("EqualityExpressionR ->");
            return lhsExprNode;
        }
        default:
            throw ParserException(
                "EqualityExpressionR", lexer.peek(),
                {TokenType::EQUAL, TokenType::NOT_EQUAL, TokenType::AND, TokenType::OR, TokenType::RIGHT_BRACKET,
                 TokenType::THEN, TokenType::DO, TokenType::TO, TokenType::DOWNTO, TokenType::RIGHT_PAREN,
                 TokenType::COMMA, TokenType::SEMICOLON, TokenType::END, TokenType::ELSE});
    }
}

Token Parser::parseEqualityOperator() {
    switch (lexer.peek().getType()) {
        case TokenType::EQUAL: {
            report("EqualityOperator -> <EQUAL>");
            return match(TokenType::EQUAL, "EqualityOperator");
        }
        case TokenType::NOT_EQUAL: {
            report("EqualityOperator -> <NOT_EQUAL>");
            return match(TokenType::NOT_EQUAL, "EqualityOperator");
        }
        default:
            throw ParserException("EqualityOperator", lexer.peek(), {TokenType::EQUAL, TokenType::NOT_EQUAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseRelationalExpression() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::IDENTIFIER:
        case TokenType::LEFT_PAREN:
        case TokenType::INTEGER_LITERAL:
        case TokenType::REAL_LITERAL: {
            report("RelationalExpression -> AdditiveExpression RelationalExpressionR");
            auto lhsExprNode = parseAdditiveExpression();
            return parseRelationalExpressionR(std::move(lhsExprNode));
        }
        default:
            throw ParserException("RelationalExpression", lexer.peek(),
                                  {TokenType::MINUS, TokenType::NOT, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
                                   TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseRelationalExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode) {
    switch (lexer.peek().getType()) {
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL: {
            report("RelationalExpressionR -> RelationalOperator AdditiveExpression RelationalExpressionR");
            auto op = parseRelationalOperator();
            auto rhsExprNode = parseAdditiveExpression();
            auto newLhsExprNode = std::make_unique<BinOpASTNode>(op, std::move(lhsExprNode), std::move(rhsExprNode));
            return parseRelationalExpressionR(std::move(newLhsExprNode));
        }
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL:
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::RIGHT_BRACKET:
        case TokenType::THEN:
        case TokenType::DO:
        case TokenType::TO:
        case TokenType::DOWNTO:
        case TokenType::RIGHT_PAREN:
        case TokenType::COMMA:
        case TokenType::SEMICOLON:
        case TokenType::END:
        case TokenType::ELSE: {
            report("RelationalExpressionR ->");
            return lhsExprNode;
        }
        default:
            throw ParserException(
                "RelationalExpressionR", lexer.peek(),
                {TokenType::LESS, TokenType::LESS_EQUAL, TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::EQUAL,
                 TokenType::NOT_EQUAL, TokenType::AND, TokenType::OR, TokenType::RIGHT_BRACKET, TokenType::THEN,
                 TokenType::DO, TokenType::TO, TokenType::DOWNTO, TokenType::RIGHT_PAREN, TokenType::COMMA,
                 TokenType::SEMICOLON, TokenType::END, TokenType::ELSE});
    }
}

Token Parser::parseRelationalOperator() {
    switch (lexer.peek().getType()) {
        case TokenType::LESS: {
            report("RelationalOperator -> <LESS>");
            return match(TokenType::LESS, "RelationalOperator");
        }
        case TokenType::LESS_EQUAL: {
            report("RelationalOperator -> <LESS_EQUAL>");
            return match(TokenType::LESS_EQUAL, "RelationalOperator");
        }
        case TokenType::GREATER: {
            report("RelationalOperator -> <GREATER>");
            return match(TokenType::GREATER, "RelationalOperator");
        }
        case TokenType::GREATER_EQUAL: {
            report("RelationalOperator -> <GREATER_EQUAL>");
            return match(TokenType::GREATER_EQUAL, "RelationalOperator");
        }
        default:
            throw ParserException(
                "RelationalOperator", lexer.peek(),
                {TokenType::LESS, TokenType::LESS_EQUAL, TokenType::GREATER, TokenType::GREATER_EQUAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseAdditiveExpression() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::IDENTIFIER:
        case TokenType::LEFT_PAREN:
        case TokenType::INTEGER_LITERAL:
        case TokenType::REAL_LITERAL: {
            report("AdditiveExpression -> MultiplicativeExpression AdditiveExpressionR");
            auto lhsExprNode = parseMultiplicativeExpression();
            return parseAdditiveExpressionR(std::move(lhsExprNode));
        }
        default:
            throw ParserException("AdditiveExpression", lexer.peek(),
                                  {TokenType::MINUS, TokenType::NOT, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
                                   TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseAdditiveExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode) {
    switch (lexer.peek().getType()) {
        case TokenType::PLUS:
        case TokenType::MINUS: {
            report("AdditiveExpressionR -> AdditiveOperator MultiplicativeExpression AdditiveExpressionR");
            auto op = parseAdditiveOperator();
            auto rhsExprNode = parseMultiplicativeExpression();
            auto newLhsExprNode = std::make_unique<BinOpASTNode>(op, std::move(lhsExprNode), std::move(rhsExprNode));
            return parseAdditiveExpressionR(std::move(newLhsExprNode));
        }
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL:
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::RIGHT_BRACKET:
        case TokenType::THEN:
        case TokenType::DO:
        case TokenType::TO:
        case TokenType::DOWNTO:
        case TokenType::RIGHT_PAREN:
        case TokenType::COMMA:
        case TokenType::SEMICOLON:
        case TokenType::END:
        case TokenType::ELSE: {
            report("AdditiveExpressionR ->");
            return lhsExprNode;
        }
        default:
            throw ParserException("AdditiveExpressionR", lexer.peek(), {TokenType::PLUS,
                                                                        TokenType::MINUS,
                                                                        TokenType::LESS,
                                                                        TokenType::LESS_EQUAL,
                                                                        TokenType::GREATER,
                                                                        TokenType::GREATER_EQUAL,
                                                                        TokenType::EQUAL,
                                                                        TokenType::NOT_EQUAL,
                                                                        TokenType::AND,
                                                                        TokenType::OR,
                                                                        TokenType::RIGHT_BRACKET,
                                                                        TokenType::THEN,
                                                                        TokenType::DO,
                                                                        TokenType::TO,
                                                                        TokenType::DOWNTO,
                                                                        TokenType::RIGHT_PAREN,
                                                                        TokenType::COMMA,
                                                                        TokenType::SEMICOLON,
                                                                        TokenType::END,
                                                                        TokenType::ELSE});
    }
}

Token Parser::parseAdditiveOperator() {
    switch (lexer.peek().getType()) {
        case TokenType::PLUS: {
            report("AdditiveOperator -> <PLUS>");
            return match(TokenType::PLUS, "AdditiveOperator");
        }
        case TokenType::MINUS: {
            report("AdditiveOperator -> <MINUS>");
            return match(TokenType::MINUS, "AdditiveOperator");
        }
        default:
            throw ParserException("AdditiveOperator", lexer.peek(), {TokenType::PLUS, TokenType::MINUS});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseMultiplicativeExpression() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::IDENTIFIER:
        case TokenType::LEFT_PAREN:
        case TokenType::INTEGER_LITERAL:
        case TokenType::REAL_LITERAL: {
            report("MultiplicativeExpression -> UnaryExpression MultiplicativeExpressionR");
            auto lhsExprNode = parseUnaryExpression();
            return parseMultiplicativeExpressionR(std::move(lhsExprNode));
        }
        default:
            throw ParserException("MultiplicativeExpression", lexer.peek(),
                                  {TokenType::MINUS, TokenType::NOT, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
                                   TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseMultiplicativeExpressionR(std::unique_ptr<ExprASTNode> lhsExprNode) {
    switch (lexer.peek().getType()) {
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::MOD:
        case TokenType::DIV: {
            report("MultiplicativeExpressionR -> MultiplicativeOperator UnaryExpression MultiplicativeExpressionR");
            auto op = parseMultiplicativeOperator();
            auto rhsExprNode = parseUnaryExpression();
            auto newLhsExprNode = std::make_unique<BinOpASTNode>(op, std::move(lhsExprNode), std::move(rhsExprNode));
            return parseMultiplicativeExpressionR(std::move(newLhsExprNode));
        }
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL:
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::RIGHT_BRACKET:
        case TokenType::THEN:
        case TokenType::DO:
        case TokenType::TO:
        case TokenType::DOWNTO:
        case TokenType::RIGHT_PAREN:
        case TokenType::COMMA:
        case TokenType::SEMICOLON:
        case TokenType::END:
        case TokenType::ELSE: {
            report("MultiplicativeExpressionR ->");
            return lhsExprNode;
        }
        default:
            throw ParserException("MultiplicativeExpressionR", lexer.peek(),
                                  {TokenType::MULTIPLY,      TokenType::DIVIDE,      TokenType::MOD,
                                   TokenType::DIV,           TokenType::PLUS,        TokenType::MINUS,
                                   TokenType::LESS,          TokenType::LESS_EQUAL,  TokenType::GREATER,
                                   TokenType::GREATER_EQUAL, TokenType::EQUAL,       TokenType::NOT_EQUAL,
                                   TokenType::AND,           TokenType::OR,          TokenType::RIGHT_BRACKET,
                                   TokenType::THEN,          TokenType::DO,          TokenType::TO,
                                   TokenType::DOWNTO,        TokenType::RIGHT_PAREN, TokenType::COMMA,
                                   TokenType::SEMICOLON,     TokenType::END,         TokenType::ELSE});
    }
}

Token Parser::parseMultiplicativeOperator() {
    switch (lexer.peek().getType()) {
        case TokenType::MULTIPLY: {
            report("MultiplicativeOperator -> <MULTIPLY>");
            return match(TokenType::MULTIPLY, "MultiplicativeOperator");
        }
        case TokenType::DIVIDE: {
            report("MultiplicativeOperator -> <DIVIDE>");
            return match(TokenType::DIVIDE, "MultiplicativeOperator");
        }
        case TokenType::MOD: {
            report("MultiplicativeOperator -> <MOD>");
            return match(TokenType::MOD, "MultiplicativeOperator");
        }
        case TokenType::DIV: {
            report("MultiplicativeOperator -> <DIV>");
            return match(TokenType::DIV, "MultiplicativeOperator");
        }
        default:
            throw ParserException("MultiplicativeOperator", lexer.peek(),
                                  {TokenType::MULTIPLY, TokenType::DIVIDE, TokenType::MOD, TokenType::DIV});
    }
}

std::unique_ptr<ExprASTNode> Parser::parseUnaryExpression() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS:
        case TokenType::NOT: {
            report("UnaryExpression -> UnaryOperator UnaryExpression");
            auto op = parseUnaryOperator();
            auto exprNode = parseUnaryExpression();
            return std::make_unique<UnaryOpASTNode>(op, std::move(exprNode));
        }
        case TokenType::IDENTIFIER:
        case TokenType::LEFT_PAREN:
        case TokenType::INTEGER_LITERAL:
        case TokenType::REAL_LITERAL: {
            report("UnaryExpression -> PrimaryExpression");
            return parsePrimaryExpression();
        }
        default:
            throw ParserException("UnaryExpression", lexer.peek(),
                                  {TokenType::MINUS, TokenType::NOT, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
                                   TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL});
    }
}

Token Parser::parseUnaryOperator() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS: {
            report("UnaryOperator -> <MINUS>");
            return match(TokenType::MINUS, "UnaryOperator");
        }
        case TokenType::NOT: {
            report("UnaryOperator -> <NOT>");
            return match(TokenType::NOT, "UnaryOperator");
        }
        default:
            throw ParserException("UnaryOperator", lexer.peek(), {TokenType::MINUS, TokenType::NOT});
    }
}

std::unique_ptr<ExprASTNode> Parser::parsePrimaryExpression() {
    switch (lexer.peek().getType()) {
        case TokenType::IDENTIFIER: {
            report("PrimaryExpression -> <IDENTIFIER> PrimaryExpressionIdentifierContinuation");
            auto identifierToken = match(TokenType::IDENTIFIER, "PrimaryExpression");
            std::string identifier = std::get<std::string>(identifierToken.getValue().value());
            return parsePrimaryExpressionIdentifierContinuation(identifier);
        }
        case TokenType::LEFT_PAREN: {
            report("PrimaryExpression -> <LEFT_PAREN> Expression <RIGHT_PAREN>");
            match(TokenType::LEFT_PAREN, "PrimaryExpression");
            auto exprNode = parseExpression();
            match(TokenType::RIGHT_PAREN, "PrimaryExpression");
            return exprNode;
        }
        case TokenType::INTEGER_LITERAL:
        case TokenType::REAL_LITERAL: {
            report("PrimaryExpression -> UnsignedNumber");
            auto numToken = parseUnsignedNumber();
            return std::make_unique<LiteralASTNode>(numToken.getValue().value());
        }
        default:
            throw ParserException(
                "PrimaryExpression", lexer.peek(),
                {TokenType::IDENTIFIER, TokenType::LEFT_PAREN, TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL});
    }
}

std::unique_ptr<ExprASTNode> Parser::parsePrimaryExpressionIdentifierContinuation(const std::string& identifier) {
    switch (lexer.peek().getType()) {
        case TokenType::LEFT_PAREN: {
            report("PrimaryExpressionIdentifierContinuation -> FunctionArgs");
            return std::make_unique<FunCallASTNode>(identifier, parseFunctionArgs());
        }
        case TokenType::LEFT_BRACKET: {
            report("PrimaryExpressionIdentifierContinuation -> ArrayAccess");
            return parseArrayAccess(identifier);
        }
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::MOD:
        case TokenType::DIV:
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL:
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::SEMICOLON:
        case TokenType::RIGHT_BRACKET:
        case TokenType::THEN:
        case TokenType::DO:
        case TokenType::TO:
        case TokenType::DOWNTO:
        case TokenType::RIGHT_PAREN:
        case TokenType::COMMA:
        case TokenType::END:
        case TokenType::ELSE: {
            report("PrimaryExpressionIdentifierContinuation ->");
            return std::make_unique<DeclVarRefASTNode>(identifier);
        }
        default:
            throw ParserException(
                "PrimaryExpressionIdentifierContinuation", lexer.peek(),
                {TokenType::LEFT_PAREN, TokenType::LEFT_BRACKET,  TokenType::MULTIPLY,    TokenType::DIVIDE,
                 TokenType::MOD,        TokenType::DIV,           TokenType::PLUS,        TokenType::MINUS,
                 TokenType::LESS,       TokenType::LESS_EQUAL,    TokenType::GREATER,     TokenType::GREATER_EQUAL,
                 TokenType::EQUAL,      TokenType::NOT_EQUAL,     TokenType::AND,         TokenType::OR,
                 TokenType::SEMICOLON,  TokenType::RIGHT_BRACKET, TokenType::THEN,        TokenType::DO,
                 TokenType::TO,         TokenType::DOWNTO,        TokenType::RIGHT_PAREN, TokenType::COMMA,
                 TokenType::END,        TokenType::ELSE});
    }
}

std::vector<std::unique_ptr<ExprASTNode>> Parser::parseFunctionArgs() {
    switch (lexer.peek().getType()) {
        case TokenType::LEFT_PAREN: {
            report("FunctionArgs -> <LEFT_PAREN> ArgumentList <RIGHT_PAREN>");
            match(TokenType::LEFT_PAREN, "FunctionArgs");
            auto paramNodes = parseArgumentList();
            match(TokenType::RIGHT_PAREN, "FunctionArgs");
            return paramNodes;
        }
        default:
            throw ParserException("FunctionArgs", lexer.peek(), {TokenType::LEFT_PAREN});
    }
}

std::vector<std::unique_ptr<ExprASTNode>> Parser::parseArgumentList() {
    switch (lexer.peek().getType()) {
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::IDENTIFIER:
        case TokenType::LEFT_PAREN:
        case TokenType::REAL_LITERAL:
        case TokenType::INTEGER_LITERAL: {
            report("ArgumentList -> Expression ArgumentListR");
            std::vector<std::unique_ptr<ExprASTNode>> argNodes;
            argNodes.push_back(parseExpression());
            parseArgumentListR(argNodes);
            return argNodes;
        }
        case TokenType::RIGHT_PAREN: {
            report("ArgumentList ->");
            return {};
        }
        default:
            throw ParserException("ArgumentList", lexer.peek(),
                                  {TokenType::MINUS, TokenType::NOT, TokenType::IDENTIFIER, TokenType::LEFT_PAREN,
                                   TokenType::REAL_LITERAL, TokenType::INTEGER_LITERAL, TokenType::RIGHT_PAREN});
    }
}

void Parser::parseArgumentListR(std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    switch (lexer.peek().getType()) {
        case TokenType::COMMA: {
            report("ArgumentListR -> <COMMA> Expression ArgumentListR");
            match(TokenType::COMMA, "ArgumentListR");
            argNodes.push_back(parseExpression());
            parseArgumentListR(argNodes);
            break;
        }
        case TokenType::RIGHT_PAREN: {
            report("ArgumentListR ->");
            break;
        }
        default:
            throw ParserException("ArgumentListR", lexer.peek(), {TokenType::COMMA, TokenType::RIGHT_PAREN});
    }
}

ParserException::ParserException(const std::string& rule, const Token& actualToken,
                                 const std::set<TokenType>& expectedTokenTypes) {
    std::ostringstream oss;
    oss << "Rule " << rule << " at position " << actualToken.getPosition() << ". ";
    oss << "Actual token was: " << actualToken.getType() << ". ";

    if (expectedTokenTypes.size() > 1) {
        oss << "Expected one of ";
        std::copy(expectedTokenTypes.begin(), expectedTokenTypes.end(),
                  std::experimental::make_ostream_joiner(oss, ", "));
        oss << ".";
    } else if (expectedTokenTypes.size() == 1) {
        oss << "Expected: " << *expectedTokenTypes.begin() << ". ";
    }

    message = oss.str();
}

const char* ParserException::what() const noexcept {
    return message.c_str();
}
