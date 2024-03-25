#pragma once
#include "DefaultVisitor.hpp"
#include "ast/CodeGenerator.hpp"

/**
 * @brief Visitor for generating LLVM types
 * @note ! Only for type nodes
 */
class GenTypeVisitor : public DefaultVisitor {
   private:
    GenContext& gen;
    llvm::Type* type = nullptr;

   public:
    explicit GenTypeVisitor(GenContext& gen);

    [[nodiscard]] llvm::Type* getType() const;

    void visit(PrimitiveTypeASTNode& node) override;
    void visit(ArrayTypeASTNode& node) override;
};
