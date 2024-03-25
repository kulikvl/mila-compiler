#pragma once
#include "AST.hpp"

// Forward declaration
struct GenContext;

/**
 * @brief Abstract function call handler
 */
class FuncHandler {
   protected:
    GenContext& gen;
    std::shared_ptr<FuncHandler> nextHandler;

   public:
    virtual ~FuncHandler() = default;

    explicit FuncHandler(GenContext& gen);
    std::shared_ptr<FuncHandler> setNext(std::shared_ptr<FuncHandler> next);

    /**
     * @brief Handle function call
     * @param funName Function name
     * @param argNodes Function arguments
     * @returns Function call return value
     */
    virtual llvm::Value* handle(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes);

    virtual void validateCall(const std::string& funName,
                              const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) = 0;
};

class WriteFuncHandler : public FuncHandler {
   public:
    explicit WriteFuncHandler(GenContext& gen);

    void validateCall(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
    llvm::Value* handle(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
};

class WritelnFuncHandler : public FuncHandler {
   public:
    explicit WritelnFuncHandler(GenContext& gen);

    void validateCall(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
    llvm::Value* handle(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
};

class ReadlnFuncHandler : public FuncHandler {
   public:
    explicit ReadlnFuncHandler(GenContext& gen);

    void validateCall(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
    llvm::Value* handle(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
};

class ToIntegerFuncHandler : public FuncHandler {
   public:
    explicit ToIntegerFuncHandler(GenContext& gen);

    void validateCall(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
    llvm::Value* handle(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
};

class ToRealFuncHandler : public FuncHandler {
   public:
    explicit ToRealFuncHandler(GenContext& gen);

    void validateCall(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
    llvm::Value* handle(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
};

/**
 * @brief User-defined function/procedure call handler
 */
class UserFuncHandler : public FuncHandler {
   public:
    explicit UserFuncHandler(GenContext& gen);

    void validateCall(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
    llvm::Value* handle(const std::string& funName, const std::vector<std::unique_ptr<ExprASTNode>>& argNodes) override;
};
