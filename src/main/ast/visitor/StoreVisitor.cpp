#include "StoreVisitor.hpp"
#include "CodeGenVisitor.hpp"
#include "GenTypeVisitor.hpp"
#include "utils/Utils.hpp"

StoreVisitor::StoreVisitor(GenContext& gen) : gen(gen) {}

llvm::Value* StoreVisitor::getStore() const {
    return store;
}

void StoreVisitor::visit(DeclVarRefASTNode& node) {
    if (!gen.symbolTable.contains(node.getRefName()))
        throw CodeGenException("Variable/Constant not found: " + node.getRefName());

    const Symbol& symbol = gen.symbolTable.getSymbol(node.getRefName());

    if (symbol.isGlobal())
        store = symbol.getGlobalMemPtr();
    else
        store = symbol.getLocalMemPtr();
}

void StoreVisitor::visit(DeclArrayRefASTNode& node) {
    if (!gen.symbolTable.contains(node.getRefName()))
        throw CodeGenException("Array not found: " + node.getRefName());

    const Symbol& symbol = gen.symbolTable.getSymbol(node.getRefName());

    CodeGenVisitor codeGenVisitor(gen);
    node.getIndexNode()->accept(codeGenVisitor);
    llvm::Value* indexV = codeGenVisitor.getValue();

    if (!indexV)
        throw CodeGenException("Array index value is not found: " + node.getRefName());

    if (!indexV->getType()->isIntegerTy())
        throw CodeGenException("Array index value is not an integer: " + node.getRefName());

    // Safe cast, because we checked that it's an array
    auto* arrType = dynamic_cast<const ArrayTypeASTNode*>(symbol.type);
    auto* lowerBoundV = llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), arrType->getLowerBound());
    auto* upperBoundV = llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), arrType->getUpperBound());

    // Check if the index is within bounds
    Utils::LLVM::generateIndexOutOfBoundsCheck(node.getRefName(), indexV, lowerBoundV, upperBoundV, gen);

    // Handles non-zero based arrays
    auto* adjustedIndexV = gen.builder.CreateSub(indexV, lowerBoundV, "adjustedIndex");

    // Get array type
    GenTypeVisitor genTypeVisitor(gen);
    symbol.type->accept(genTypeVisitor);
    auto* llvmArrayType = genTypeVisitor.getType();
    //    auto* llvmElementType = llvmArrayType->getArrayElementType();

    // Calculate the address of the element at the given index
    std::vector<llvm::Value*> indices = {gen.builder.getInt32(0),
                                         adjustedIndexV};  // For accessing an element in an array

    llvm::Value* elementV =
        (symbol.isGlobal())
            ? gen.builder.CreateGEP(llvmArrayType, symbol.getGlobalMemPtr(), indices, node.getRefName() + "_idx")
            : gen.builder.CreateGEP(llvmArrayType, symbol.getLocalMemPtr(), indices, node.getRefName() + "_idx");

    store = elementV;
}
