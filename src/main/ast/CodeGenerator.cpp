#include "CodeGenerator.hpp"
#include "ast/visitor/CodeGenVisitor.hpp"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

bool Symbol::isGlobal() const {
    return std::holds_alternative<llvm::GlobalVariable*>(memPtr);
}

llvm::GlobalVariable* Symbol::getGlobalMemPtr() const {
    if (!isGlobal())
        throw std::runtime_error("Failed to get global memory pointer - symbol is not global");

    return std::get<llvm::GlobalVariable*>(memPtr);
}

llvm::AllocaInst* Symbol::getLocalMemPtr() const {
    if (isGlobal())
        throw std::runtime_error("Failed to get local (stack) memory pointer - symbol is global");

    return std::get<llvm::AllocaInst*>(memPtr);
}

void SymbolTable::addSymbol(const std::string& name, const Symbol& symbol) {
    if (contains(name))
        throw CodeGenException("Failed to add new symbol - symbol already exists: " + name);

    table[name] = symbol;
}

const Symbol& SymbolTable::getSymbol(const std::string& name) const {
    if (!contains(name))
        throw CodeGenException("Failed to get symbol - symbol not found: " + name);

    return table.at(name);
}

bool SymbolTable::contains(const std::string& name) const {
    return table.count(name) > 0;
}

GenContext::GenContext(const std::string& moduleName) : ctx(), builder(ctx), module(moduleName, ctx) {
    auto writeHandler = std::make_shared<WriteFuncHandler>(*this);
    auto writelnHandler = std::make_shared<WritelnFuncHandler>(*this);
    auto readlnHandler = std::make_shared<ReadlnFuncHandler>(*this);
    auto toIntegerHandler = std::make_shared<ToIntegerFuncHandler>(*this);
    auto toRealHandler = std::make_shared<ToRealFuncHandler>(*this);
    auto userHandler = std::make_shared<UserFuncHandler>(*this);

    writeHandler->setNext(writelnHandler)
        ->setNext(readlnHandler)
        ->setNext(toIntegerHandler)
        ->setNext(toRealHandler)
        ->setNext(userHandler);

    funcHandler = writeHandler;
}

CodeGenerator::CodeGenerator(ASTNode* astNode) : astNode(astNode) {}

void CodeGenerator::generate() const {
    GenContext gen("mila-module");
    CodeGenVisitor codegenVisitor(gen);
    astNode->accept(codegenVisitor);
    gen.module.print(llvm::outs(), nullptr);
}

void CodeGenerator::generate(const std::string& outFile) const {
    GenContext gen("mila-module");
    CodeGenVisitor codegenVisitor(gen);
    astNode->accept(codegenVisitor);

    std::error_code EC;
    llvm::raw_fd_ostream out(outFile, EC, llvm::sys::fs::OF_None);

    if (EC)
        throw CodeGenException("Failed to open file: " + outFile);

    gen.module.print(out, nullptr);
}

CodeGenException::CodeGenException(std::string msg) : message(std::move(msg)) {}

const char* CodeGenException::what() const noexcept {
    return message.c_str();
}
