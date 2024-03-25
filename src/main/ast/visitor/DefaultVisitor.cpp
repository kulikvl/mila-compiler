#include "DefaultVisitor.hpp"

DefaultVisitor::DefaultVisitor(bool throws) : throws(throws) {}

void DefaultVisitor::visit() const {
    if (throws)
        throw std::runtime_error("DefaultVisitor: Method not implemented");
}

void DefaultVisitor::visit([[maybe_unused]] PrimitiveTypeASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] ArrayTypeASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] BinOpASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] UnaryOpASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] LiteralASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] DeclVarRefASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] DeclArrayRefASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] FunCallASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] BlockASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] CompoundStmtASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] VarDeclASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] ArrayDeclASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] ConstDefASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] ProcDeclASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] FunDeclASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] AssignASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] IfASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] WhileASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] ForASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] ProcCallASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] EmptyStmtASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] ProgramASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] BreakASTNode& node) {
    visit();
}

void DefaultVisitor::visit([[maybe_unused]] ExitASTNode& node) {
    visit();
}
