#include "GenTypeVisitor.hpp"

GenTypeVisitor::GenTypeVisitor(GenContext& gen) : gen(gen) {}

llvm::Type* GenTypeVisitor::getType() const {
    return type;
}

void GenTypeVisitor::visit(PrimitiveTypeASTNode& node) {
    switch (node.getPrimitiveType()) {
        case PrimitiveTypeASTNode::PrimitiveType::REAL:
            type = llvm::Type::getDoubleTy(gen.ctx);
            break;
        case PrimitiveTypeASTNode::PrimitiveType::INTEGER:
            type = llvm::Type::getInt32Ty(gen.ctx);
            break;
        default:
            throw CodeGenException("Unknown Primitive type");
    }
}

void GenTypeVisitor::visit(ArrayTypeASTNode& node) {
    node.getElemTypeNode()->accept(*this);
    llvm::Type* elementType = type;
    uint64_t size = node.getUpperBound() - node.getLowerBound() + 1;
    type = llvm::ArrayType::get(elementType, size);
}
