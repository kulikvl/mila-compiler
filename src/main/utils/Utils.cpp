#include "Utils.hpp"

namespace Utils {

ProgramRunResult::ProgramRunResult(int exitCode, std::string output) : exitCode(exitCode), output(std::move(output)) {}

ProgramRunResult exec(const std::string& shellCmd) {
    std::string result;

    FILE* pipe = popen(shellCmd.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("popen() failed!");

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        result += buffer;

    int status = pclose(pipe);
    if (status == -1)
        throw std::runtime_error("pclose() failed!");

    if (!WIFEXITED(status))
        throw std::runtime_error("Shell command did not terminate normally!");

    return {WEXITSTATUS(status), result};
}

std::vector<std::string> convertArgvToVector(int argc, char* argv[]) {
    if (argc <= 1)
        return {};

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
        args.emplace_back(argv[i]);

    return args;
}

bool hasExtension(const std::string& filename, const std::string& extension) {
    if (filename.length() <= extension.length())
        return false;

    return filename.compare(filename.length() - extension.length(), extension.length(), extension) == 0;
}

namespace LLVM {

void generateIndexOutOfBoundsCheck(const std::string& arrayName, llvm::Value* indexV, llvm::Value* lowerBoundV,
                                   llvm::Value* upperBoundV, GenContext& gen) {
    auto* indexOutOfBounds = gen.builder.CreateOr(gen.builder.CreateICmpSLT(indexV, lowerBoundV),
                                                  gen.builder.CreateICmpSGT(indexV, upperBoundV), "indexOutOfBounds");

    auto func = gen.builder.GetInsertBlock()->getParent();
    if (!func)
        throw CodeGenException("generateIndexOutOfBoundsCheck(): Parent function is not found");
    llvm::BasicBlock* CheckBlock = llvm::BasicBlock::Create(gen.ctx, "check", func);
    llvm::BasicBlock* ExceptionBlock = llvm::BasicBlock::Create(gen.ctx, "throw_exception", func);
    llvm::BasicBlock* ContinueBlock = llvm::BasicBlock::Create(gen.ctx, "continue", func);

    // Emit code that checks if index is out of bounds
    gen.builder.CreateBr(CheckBlock);
    gen.builder.SetInsertPoint(CheckBlock);
    gen.builder.CreateCondBr(indexOutOfBounds, ExceptionBlock, ContinueBlock);

    // Handle index out of bounds exception with error message printing to user
    gen.builder.SetInsertPoint(ExceptionBlock);
    llvm::Value* errMsg =
        gen.builder.CreateGlobalStringPtr("Runtime error: Array '" + arrayName + "' - the index is out of bounds.\n");

    // Declare error (printf + exit in C) function in-place here, because the user should not be able to call, and it terminates the program execution
    auto* errorFunc = gen.module.getFunction("error");  // check if error function has already been declared
    if (!errorFunc) {
        std::vector<llvm::Type*> errorFuncArgs = {llvm::PointerType::get(llvm::Type::getInt8Ty(gen.ctx), 0)};
        llvm::FunctionType* errorFuncType =
            llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(gen.ctx), errorFuncArgs, true);
        errorFunc = llvm::Function::Create(errorFuncType, llvm::Function::ExternalLinkage, "error", gen.module);
    }

    gen.builder.CreateCall(errorFunc, {errMsg});
    // here it terminates
    gen.builder.CreateUnreachable();

    // Continue with normal execution
    gen.builder.SetInsertPoint(ContinueBlock);
}

}  // namespace LLVM

}  // namespace Utils
