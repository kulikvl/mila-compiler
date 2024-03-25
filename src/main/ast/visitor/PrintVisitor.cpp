#include "PrintVisitor.hpp"

PrintVisitor::PrintVisitor(std::ostream& os, int indent) : os(os), indent(indent) {}

#define MKINDENT os << mkIndent(indent);

std::ostream& operator<<(std::ostream& os, PrimitiveTypeASTNode::PrimitiveType type) {
    switch (type) {
        case PrimitiveTypeASTNode::PrimitiveType::REAL:
            return os << "real";
        case PrimitiveTypeASTNode::PrimitiveType::INTEGER:
            return os << "integer";
        default:
            throw std::logic_error("Invalid type");
    }
}

std::string mkIndent(int cnt) {
    std::ostringstream oss;
    while (cnt--)
        oss << ' ' << ' ';
    return oss.str();
}

void PrintVisitor::visit(PrimitiveTypeASTNode& node) {
    MKINDENT;
    os << "PrimitiveType " << node.getPrimitiveType() << "\n";
}

void PrintVisitor::visit(ArrayTypeASTNode& node) {
    MKINDENT;
    os << "ArrayType of " << node.getElemTypeNode()->getPrimitiveType() << " from " << node.getLowerBound() << " to "
       << node.getUpperBound() << "\n";
}

void PrintVisitor::visit(BinOpASTNode& node) {
    MKINDENT;
    os << "BinOp " << node.getOp() << "\n";
    indent++;
    node.getLhsExprNode()->accept(*this);
    node.getRhsExprNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit(UnaryOpASTNode& node) {
    MKINDENT;
    os << "UnaryOp " << node.getOp() << "\n";
    indent++;
    node.getExprNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit(LiteralASTNode& node) {
    MKINDENT;
    os << "Literal " << node.getValue() << "\n";
}

void PrintVisitor::visit(DeclVarRefASTNode& node) {
    MKINDENT;
    os << "DeclVarRef " << node.getRefName() << "\n";
}

void PrintVisitor::visit(DeclArrayRefASTNode& node) {
    MKINDENT;
    os << "DeclArrayRef " << node.getRefName() << "\n";
    indent++;
    node.getIndexNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit(FunCallASTNode& node) {
    MKINDENT;
    os << "FunCall " << node.getFunName() << "\n";
    indent++;
    for (const auto& arg : node.getArgNodes())
        arg->accept(*this);
    indent--;
}

void PrintVisitor::visit(BlockASTNode& node) {
    MKINDENT;
    os << "Block\n";
    indent++;
    for (const auto& s : node.getStatementNodes())
        s->accept(*this);
    indent--;
}

void PrintVisitor::visit(CompoundStmtASTNode& node) {
    MKINDENT;
    os << "CompoundStmt\n";
    indent++;
    for (const auto& s : node.getStatementNodes())
        s->accept(*this);
    indent--;
}

void PrintVisitor::visit(VarDeclASTNode& node) {
    MKINDENT;
    os << "VarDecl " << node.getDeclName() << "\n";
    indent++;
    node.getTypeNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit(ArrayDeclASTNode& node) {
    MKINDENT;
    os << "ArrayDecl " << node.getDeclName() << "\n";
    indent++;
    node.getTypeNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit(ConstDefASTNode& node) {
    MKINDENT;
    os << "ConstDef " << node.getDeclName() << "\n";
    indent++;
    if (node.getTypeNode().has_value()) node.getTypeNode().value()->accept(*this);
    indent--;
}

void PrintVisitor::visit(ProcDeclASTNode& node) {
    MKINDENT;
    os << "ProcDecl " << node.getDeclName() << "\n";
    indent++;
    for (const auto& arg : node.getParamNodes())
        arg->accept(*this);
    if (node.getBlockNode().has_value())
        node.getBlockNode().value()->accept(*this);
    else
        os << "Forward\n";
    indent--;
}

void PrintVisitor::visit(FunDeclASTNode& node) {
    MKINDENT;
    os << "FunDecl " << node.getDeclName() << "\n";
    indent++;
    for (const auto& arg : node.getParamNodes())
        arg->accept(*this);
    if (node.getBlockNode().has_value())
        node.getBlockNode().value()->accept(*this);
    else
        os << "Forward\n";
    node.getRetTypeNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit(AssignASTNode& node) {
    MKINDENT;
    os << "Assign\n";
    indent++;
    node.getVarNode()->accept(*this);
    node.getExprNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit(IfASTNode& node) {
    MKINDENT;
    os << "If\n";
    indent++;
    node.getCondNode()->accept(*this);
    indent--;
    MKINDENT;
    os << "Then\n";
    indent++;
    node.getBodyNode()->accept(*this);
    indent--;
    if (node.getElseBodyNode()) {
        MKINDENT;
        os << "Else\n";
        indent++;
        node.getElseBodyNode().value()->accept(*this);
        indent--;
    } else {
        MKINDENT;
        os << "<no-else>\n";
    }
}

void PrintVisitor::visit(WhileASTNode& node) {
    MKINDENT;
    os << "While\n";
    indent++;
    node.getCondNode()->accept(*this);
    node.getBodyNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit(ForASTNode& node) {
    MKINDENT;
    os << "For\n";
    indent++;
    node.getInitNode()->accept(*this);
    node.getToNode()->accept(*this);
    node.getBodyNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit(ProcCallASTNode& node) {
    MKINDENT;
    os << "ProcCall " << node.getProcName() << "\n";
    indent++;
    for (const auto& arg : node.getArgNodes())
        arg->accept(*this);
    indent--;
}

void PrintVisitor::visit([[maybe_unused]] EmptyStmtASTNode& node) {
    MKINDENT;
    os << "EmptyStmt\n";
}

void PrintVisitor::visit(ProgramASTNode& node) {
    MKINDENT;
    os << "Program\n";
    indent++;
    node.getBlockNode()->accept(*this);
    indent--;
}

void PrintVisitor::visit([[maybe_unused]] BreakASTNode& node) {
    MKINDENT;
    os << "Break\n";
}

void PrintVisitor::visit([[maybe_unused]] ExitASTNode& node) {
    MKINDENT;
    os << "Exit\n";
}
