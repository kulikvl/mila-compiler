#pragma once
#include "DefaultVisitor.hpp"
#include "ast/CodeGenerator.hpp"

/**
 * @brief Visitor for getting pointer to the allocated memory
 * @note ! Only for nodes that represent values in memory
 */
class StoreVisitor : public DefaultVisitor {
   private:
    GenContext& gen;

    /**
     * @brief The pointer to the allocated memory of variable (of declaration reference generally)
     */
    llvm::Value* store = nullptr;

   public:
    explicit StoreVisitor(GenContext& gen);

    [[nodiscard]] llvm::Value* getStore() const;

    void visit(DeclVarRefASTNode& node) override;

    /**
     * @note 'store' would be not be of llvm::AllocaInst* type, because elements within an array are not individually allocated with 'alloca'
     */
    void visit(DeclArrayRefASTNode& node) override;
};
