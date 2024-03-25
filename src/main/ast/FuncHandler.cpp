#include "FuncHandler.hpp"
#include "ast/visitor/CodeGenVisitor.hpp"
#include "ast/visitor/StoreVisitor.hpp"

FuncHandler::FuncHandler(GenContext& gen) : gen(gen) {}

std::shared_ptr<FuncHandler> FuncHandler::setNext(std::shared_ptr<FuncHandler> next) {
    nextHandler = next;
    return next;
}

llvm::Value* FuncHandler::handle(const std::string& funName,
                                 const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (nextHandler) {
        return nextHandler->handle(funName, argNodes);
    } else {
        throw std::runtime_error("Cannot handle function call: " + funName);
    }
}

WriteFuncHandler::WriteFuncHandler(GenContext& gen) : FuncHandler(gen) {
    // create write function for integers
    {
        llvm::FunctionType* funcType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), {llvm::Type::getInt32Ty(gen.ctx)}, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "write_int", gen.module);
    }

    // create write function for doubles
    {
        llvm::FunctionType* funcType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), {llvm::Type::getDoubleTy(gen.ctx)}, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "write_double", gen.module);
    }
}

void WriteFuncHandler::validateCall(const std::string& funName,
                                    const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "write")
        throw CodeGenException("WriteFuncHandler can only handle 'write' function");
    if (argNodes.size() != 1)
        throw CodeGenException("'write' procedure expects 1 argument, but " + std::to_string(argNodes.size()) +
                               " were provided");
}

llvm::Value* WriteFuncHandler::handle(const std::string& funName,
                                      const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "write")
        return FuncHandler::handle(funName, argNodes);

    validateCall(funName, argNodes);

    CodeGenVisitor codeGenVisitor(gen);
    argNodes[0]->accept(codeGenVisitor);
    auto* argV = codeGenVisitor.getValue();
    if (!argV)
        throw CodeGenException("Failed to get argument value of 'write' procedure");

    if (argV->getType()->isDoubleTy())
        return gen.builder.CreateCall(gen.module.getFunction("write_double"), argV);
    else if (argV->getType()->isIntegerTy())
        return gen.builder.CreateCall(gen.module.getFunction("write_int"), argV);
    else
        throw CodeGenException("Unsupported argument type for 'write' procedure");
}

WritelnFuncHandler::WritelnFuncHandler(GenContext& gen) : FuncHandler(gen) {
    // create writeln function for integers
    {
        llvm::FunctionType* funcType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), {llvm::Type::getInt32Ty(gen.ctx)}, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "writeln_int", gen.module);
    }

    // create writeln function for doubles
    {
        llvm::FunctionType* funcType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), {llvm::Type::getDoubleTy(gen.ctx)}, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "writeln_double", gen.module);
    }
}

void WritelnFuncHandler::validateCall(const std::string& funName,
                                      const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "writeln")
        throw CodeGenException("WritelnFuncHandler can only handle 'writeln' function");
    if (argNodes.size() != 1)
        throw CodeGenException("'writeln' procedure expects 1 argument, but " + std::to_string(argNodes.size()) +
                               " were provided");
}

llvm::Value* WritelnFuncHandler::handle(const std::string& funName,
                                        const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "writeln")
        return FuncHandler::handle(funName, argNodes);

    validateCall(funName, argNodes);

    CodeGenVisitor codeGenVisitor(gen);
    argNodes[0]->accept(codeGenVisitor);
    auto* argV = codeGenVisitor.getValue();
    if (!argV)
        throw CodeGenException("Failed to get argument value of 'writeln' procedure");

    if (argV->getType()->isDoubleTy())
        return gen.builder.CreateCall(gen.module.getFunction("writeln_double"), argV);
    else if (argV->getType()->isIntegerTy())
        return gen.builder.CreateCall(gen.module.getFunction("writeln_int"), argV);
    else
        throw CodeGenException("Unsupported argument type for 'writeln' procedure");
}

ReadlnFuncHandler::ReadlnFuncHandler(GenContext& gen) : FuncHandler(gen) {
    // create readln function for integers
    {
        llvm::FunctionType* funcType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), {llvm::Type::getInt32PtrTy(gen.ctx)}, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "readln_int", gen.module);
    }

    // create readln function for doubles
    {
        llvm::FunctionType* funcType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), {llvm::Type::getDoublePtrTy(gen.ctx)}, false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "readln_double", gen.module);
    }
}

void ReadlnFuncHandler::validateCall(const std::string& funName,
                                     const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "readln")
        throw CodeGenException("ReadlnFuncHandler can only handle 'readln' function");
    if (argNodes.size() != 1)
        throw CodeGenException("'readln' procedure expects 1 argument, but " + std::to_string(argNodes.size()) +
                               " were provided");
}

llvm::Value* ReadlnFuncHandler::handle(const std::string& funName,
                                       const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "readln")
        return FuncHandler::handle(funName, argNodes);

    validateCall(funName, argNodes);

    auto* declRefNode = dynamic_cast<DeclRefASTNode*>(argNodes[0].get());
    if (!declRefNode)
        throw CodeGenException("'readln' procedure failed, argument is not a variable");

    StoreVisitor storeVisitor(gen);
    declRefNode->accept(storeVisitor);
    auto* argStore = storeVisitor.getStore();
    if (!argStore)
        throw CodeGenException("'readln' procedure failed, argument store (memory location) not found");

    llvm::Type* storeType = argStore->getType();
    if (!storeType->isPointerTy())
        throw CodeGenException("'readln' procedure failed, store is not a pointer");

    llvm::Type* valType = storeType->getPointerElementType();
    if (valType->isDoubleTy())
        return gen.builder.CreateCall(gen.module.getFunction("readln_double"), argStore);
    else if (valType->isIntegerTy())
        return gen.builder.CreateCall(gen.module.getFunction("readln_int"), argStore);
    else
        throw CodeGenException("Unsupported argument type for 'readln' procedure");
}

ToIntegerFuncHandler::ToIntegerFuncHandler(GenContext& gen) : FuncHandler(gen) {}

void ToIntegerFuncHandler::validateCall(const std::string& funName,
                                     const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "to_integer")
        throw CodeGenException("ToIntegerFuncHandler can only handle 'to_integer' function");
    if (argNodes.size() != 1)
        throw CodeGenException("'to_integer' function expects 1 argument, but " + std::to_string(argNodes.size()) +
                               " were provided");
}

llvm::Value* ToIntegerFuncHandler::handle(const std::string& funName,
                                       const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "to_integer")
        return FuncHandler::handle(funName, argNodes);

    validateCall(funName, argNodes);

    CodeGenVisitor codeGenVisitor(gen);
    argNodes[0]->accept(codeGenVisitor);
    auto* argV = codeGenVisitor.getValue();
    if (!argV)
        throw CodeGenException("Failed to get argument value of 'to_integer' function");

    if (argV->getType()->isDoubleTy())
        return gen.builder.CreateFPToSI(argV, llvm::Type::getInt32Ty(gen.ctx));
    else if (argV->getType()->isIntegerTy())
        return argV;
    else
        throw CodeGenException("Unsupported argument type for 'to_integer' function");
}

ToRealFuncHandler::ToRealFuncHandler(GenContext& gen) : FuncHandler(gen) {}

void ToRealFuncHandler::validateCall(const std::string& funName,
                                        const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "to_real")
        throw CodeGenException("ToRealFuncHandler can only handle 'to_real' function");
    if (argNodes.size() != 1)
        throw CodeGenException("'to_real' function expects 1 argument, but " + std::to_string(argNodes.size()) +
                               " were provided");
}

llvm::Value* ToRealFuncHandler::handle(const std::string& funName,
                                          const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    if (funName != "to_real")
        return FuncHandler::handle(funName, argNodes);

    validateCall(funName, argNodes);

    CodeGenVisitor codeGenVisitor(gen);
    argNodes[0]->accept(codeGenVisitor);
    auto* argV = codeGenVisitor.getValue();
    if (!argV)
        throw CodeGenException("Failed to get argument value of 'to_real' function");

    if (argV->getType()->isIntegerTy())
        return gen.builder.CreateSIToFP(argV, llvm::Type::getDoubleTy(gen.ctx));
    else if (argV->getType()->isDoubleTy())
        return argV;
    else
        throw CodeGenException("Unsupported argument type for 'to_real' function");
}

UserFuncHandler::UserFuncHandler(GenContext& gen) : FuncHandler(gen) {}

void UserFuncHandler::validateCall(const std::string& funName,
                                   const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    auto* func = gen.module.getFunction(funName);

    if (!func)
        throw CodeGenException("Function/Procedure not found: " + funName);

    if (func->arg_size() != argNodes.size())
        throw CodeGenException("Function/Procedure " + funName + " expects " + std::to_string(func->arg_size()) +
                               " arguments, but " + std::to_string(argNodes.size()) + " were provided");
}

llvm::Value* UserFuncHandler::handle(const std::string& funName,
                                     const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) {
    validateCall(funName, argNodes);

    auto* func = gen.module.getFunction(funName);

    std::vector<llvm::Value*> argsV;

    CodeGenVisitor codeGenVisitor(gen);
    for (const auto& argNode : argNodes) {
        argNode->accept(codeGenVisitor);
        auto* argV = codeGenVisitor.getValue();
        if (!argV)
            throw CodeGenException("Failed to get argument value of " + funName + " function/procedure");
        argsV.push_back(argV);
    }

    return gen.builder.CreateCall(func, argsV);
}
