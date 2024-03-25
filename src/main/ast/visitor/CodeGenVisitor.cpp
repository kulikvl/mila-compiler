#include "CodeGenVisitor.hpp"
#include "CollectorVisitor.hpp"
#include "GenTypeVisitor.hpp"
#include "StoreVisitor.hpp"
#include "utils/Utils.hpp"

CodeGenVisitor::CodeGenVisitor(GenContext& gen) : gen(gen) {}

static llvm::Constant* getDefaultValueForType(llvm::Type* type, llvm::LLVMContext& ctx) {
    if (type->isIntegerTy()) {
        return llvm::ConstantInt::get(ctx, llvm::APInt(type->getIntegerBitWidth(), 0, true));
    } else if (type->isDoubleTy()) {
        return llvm::ConstantFP::get(ctx, llvm::APFloat(0.0));
    } else {
        throw CodeGenException("Unsupported element type for variable initialization");
    }
}

llvm::Value* CodeGenVisitor::getValue() const {
    return value;
}

void CodeGenVisitor::visit([[maybe_unused]] PrimitiveTypeASTNode& node) {}

void CodeGenVisitor::visit([[maybe_unused]] ArrayTypeASTNode& node) {}

void CodeGenVisitor::visit(BinOpASTNode& node) {
    node.getLhsExprNode()->accept(*this);
    auto* lhsV = value;
    node.getRhsExprNode()->accept(*this);
    auto* rhsV = value;

    if (!lhsV)
        throw CodeGenException("Left-hand side value of binary operator is not found");

    if (!rhsV)
        throw CodeGenException("Right-hand side value of binary operator is not found");

    bool dblArith = lhsV->getType()->isDoubleTy() || rhsV->getType()->isDoubleTy();

    /*
        LLVM function names decoding:
        SI = Signed Integer
        FP = Floating Point
        Div = Division
        EQ = Equal
        I = Integer
     */

    auto maybeSIToFP = [&](llvm::Value* val) {
        if (!val->getType()->isDoubleTy())
            return gen.builder.CreateSIToFP(val, llvm::Type::getDoubleTy(gen.ctx));
        return val;
    };

    switch (node.getOp().getType()) {
        case TokenType::PLUS:
            if (dblArith)
                value = gen.builder.CreateFAdd(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "fadd");
            else
                value = gen.builder.CreateAdd(lhsV, rhsV, "add");
            break;
        case TokenType::MINUS:
            if (dblArith)
                value = gen.builder.CreateFSub(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "fsub");
            else
                value = gen.builder.CreateSub(lhsV, rhsV, "sub");
            break;
        case TokenType::MULTIPLY:
            if (dblArith)
                value = gen.builder.CreateFMul(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "fmul");
            else
                value = gen.builder.CreateMul(lhsV, rhsV, "mul");
            break;
        case TokenType::DIV:
        case TokenType::DIVIDE:
            if (dblArith)
                value = gen.builder.CreateFDiv(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "fdiv");
            else
                value = gen.builder.CreateSDiv(lhsV, rhsV, "div");
            break;
        case TokenType::EQUAL:
            if (dblArith)
                value = gen.builder.CreateFCmpOEQ(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "feq");
            else
                value = gen.builder.CreateICmpEQ(lhsV, rhsV, "eq");
            break;
        case TokenType::LESS:
            if (dblArith)
                value = gen.builder.CreateFCmpOLT(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "flt");
            else
                value = gen.builder.CreateICmpSLT(lhsV, rhsV, "lt");
            break;
        case TokenType::GREATER:
            if (dblArith)
                value = gen.builder.CreateFCmpOGT(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "fgt");
            else
                value = gen.builder.CreateICmpSGT(lhsV, rhsV, "gt");
            break;
        case TokenType::NOT_EQUAL:
            if (dblArith)
                value = gen.builder.CreateFCmpONE(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "fneq");
            else
                value = gen.builder.CreateICmpNE(lhsV, rhsV, "neq");
            break;
        case TokenType::LESS_EQUAL:
            if (dblArith)
                value = gen.builder.CreateFCmpOLE(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "fle");
            else
                value = gen.builder.CreateICmpSLE(lhsV, rhsV, "le");
            break;
        case TokenType::GREATER_EQUAL:
            if (dblArith)
                value = gen.builder.CreateFCmpOGE(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "fge");
            else
                value = gen.builder.CreateICmpSGE(lhsV, rhsV, "ge");
            break;
        case TokenType::OR:
            if (dblArith)
                throw CodeGenException("Unsupported logical OR operation for real type");
            else
                value = gen.builder.CreateOr(lhsV, rhsV, "or");
            break;
        case TokenType::AND:
            if (dblArith)
                throw CodeGenException("Unsupported logical AND operation for real type");
            else
                value = gen.builder.CreateAnd(lhsV, rhsV, "and");
            break;
        case TokenType::MOD:
            if (dblArith)
                value = gen.builder.CreateFRem(maybeSIToFP(lhsV), maybeSIToFP(rhsV), "fmod");
            else
                value = gen.builder.CreateSRem(lhsV, rhsV, "mod");
            break;
        default:
            throw CodeGenException("Unknown binary operator");
    }
}

void CodeGenVisitor::visit(UnaryOpASTNode& node) {
    node.getExprNode()->accept(*this);
    auto* exprV = value;

    if (!exprV)
        throw CodeGenException("Expression value is not found");

    switch (node.getOp().getType()) {
        case TokenType::MINUS:
            if (exprV->getType()->isDoubleTy())
                value = gen.builder.CreateFNeg(exprV, "fneg");
            else
                value = gen.builder.CreateNeg(exprV, "neg");
            break;
        case TokenType::NOT:
            if (exprV->getType()->isDoubleTy())
                throw CodeGenException("Unsupported NOT operation for real type");
            else  // NOT is achieved by XOR with 1
                value = gen.builder.CreateXor(exprV, llvm::ConstantInt::get(llvm::Type::getInt1Ty(gen.ctx), 1), "not");
            break;
        default:
            throw CodeGenException("Unknown unary operator");
    }
}

void CodeGenVisitor::visit(LiteralASTNode& node) {
    TokenValue tokenValue = node.getValue();

    if (std::holds_alternative<int>(tokenValue)) {
        value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), std::get<int>(tokenValue));
    } else if (std::holds_alternative<double>(tokenValue)) {
        value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(gen.ctx), std::get<double>(tokenValue));
    } else {
        throw CodeGenException("Unknown literal type");
    }
}

void CodeGenVisitor::visit(DeclVarRefASTNode& node) {
    if (!gen.symbolTable.contains(node.getRefName()))
        throw CodeGenException("Variable/Constant not found: " + node.getRefName());

    const Symbol& symbol = gen.symbolTable.getSymbol(node.getRefName());

    // Get symbol type
    GenTypeVisitor genTypeVisitor(gen);
    symbol.type->accept(genTypeVisitor);
    auto* varType = genTypeVisitor.getType();

    // Load value
    llvm::Value* loadedValue = (symbol.isGlobal())
                                   ? gen.builder.CreateLoad(varType, symbol.getGlobalMemPtr(), node.getRefName())
                                   : gen.builder.CreateLoad(varType, symbol.getLocalMemPtr(), node.getRefName());

    value = loadedValue;
}

void CodeGenVisitor::visit(DeclArrayRefASTNode& node) {
    // Validations
    if (!gen.symbolTable.contains(node.getRefName()))
        throw CodeGenException("Array identifier not found: " + node.getRefName());

    const Symbol& symbol = gen.symbolTable.getSymbol(node.getRefName());

    if (symbol.type->getType() != TypeASTNode::Type::ARRAY)
        throw CodeGenException("Identifier is not an array: " + node.getRefName());

    node.getIndexNode()->accept(*this);
    auto* indexV = value;

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
    // Calculate the address of the element at the given index. Array access in LLVM IR is done using a zero index followed by the actual index
    std::vector<llvm::Value*> indices = {gen.builder.getInt32(0), adjustedIndexV};

    // Get array type and element type
    GenTypeVisitor genTypeVisitor(gen);
    symbol.type->accept(genTypeVisitor);
    auto* llvmArrayType = genTypeVisitor.getType();
    auto* llvmElementType = llvmArrayType->getArrayElementType();

    llvm::Value* elementPtr =
        (symbol.isGlobal())
            ? gen.builder.CreateGEP(llvmArrayType, symbol.getGlobalMemPtr(), indices, node.getRefName() + "_idx")
            : gen.builder.CreateGEP(llvmArrayType, symbol.getLocalMemPtr(), indices, node.getRefName() + "_idx");

    llvm::Value* loadedValue = gen.builder.CreateLoad(llvmElementType, elementPtr, node.getRefName() + "_elem");

    value = loadedValue;
}

void CodeGenVisitor::visit(FunCallASTNode& node) {
    auto* retVal = gen.funcHandler->handle(node.getFunName(), node.getArgNodes());
    value = retVal;
}

void CodeGenVisitor::visit(BlockASTNode& node) {
    if (node.isMain()) {
        // create main function
        llvm::FunctionType* funcTypeMain = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.ctx), false);
        llvm::Function* funcMain =
            llvm::Function::Create(funcTypeMain, llvm::Function::ExternalLinkage, "main", gen.module);

        // create entry block for main function
        llvm::BasicBlock* EntryBlock = llvm::BasicBlock::Create(gen.ctx, "entry", funcMain);
        gen.builder.SetInsertPoint(EntryBlock);

        // Find variable and constant declarations and set them as global
        for (auto& s : node.getStatementNodes()) {
            if (auto* declNode = dynamic_cast<DeclASTNode*>(s.get())) {
                declNode->setGlobal(true);
            }
        }

        // Find exit statements and set their return value to 0
        llvm::Value* retV = llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 0);
        CollectorVisitor<ExitASTNode> exitNodesVisitor;
        node.accept(exitNodesVisitor);
        for (auto exitNode : exitNodesVisitor.collectedNodes) {
            exitNode->setRetV(retV);
        }

        // Generate code for the block
        for (const auto& s : node.getStatementNodes()) {
            s->accept(*this);
        }

        gen.builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 0));
    } else {
        // func is a pointer to the function that the current insertion block belongs to.
        // The new block 'BB' will be inserted after the current block into this function.
        auto func = gen.builder.GetInsertBlock()->getParent();

        if (!func)
            throw CodeGenException("Parent function is not found");

        llvm::BasicBlock* BB = llvm::BasicBlock::Create(gen.ctx, "block", func);

        // Creates a branch instruction. This instruction unconditionally jumps to the newly created basic block 'BB'.
        // This is used to ensure that the execution flow continues into the new block from the current position in the code.
        gen.builder.CreateBr(BB);

        // Set the newly created basic block 'BB' as the current insertion point for subsequent instructions
        // (new code will be inserted into 'BB' until the insertion point is changed again).
        gen.builder.SetInsertPoint(BB);

        SymbolTable symbolTableCopy = gen.symbolTable;

        for (const auto& s : node.getStatementNodes()) {
            s->accept(*this);
        }

        // Restore the original symbol table.
        // This effectively undoes any changes to the symbol table made during the code generation of this block's statements.
        gen.symbolTable = symbolTableCopy;

        // This is just code block (statements), not an expression.
        value = nullptr;
    }
}

void CodeGenVisitor::visit(CompoundStmtASTNode& node) {
    auto func = gen.builder.GetInsertBlock()->getParent();

    if (!func)
        throw CodeGenException("Parent function is not found");

    for (const auto& s : node.getStatementNodes()) {
        s->accept(*this);
    }

    value = nullptr;
}

void CodeGenVisitor::visit(VarDeclASTNode& node) {
    if (gen.symbolTable.contains(node.getDeclName()))
        throw CodeGenException("Variable is already declared: " + node.getDeclName());

    // Get variable type
    GenTypeVisitor genTypeVisitor(gen);
    node.getTypeNode()->accept(genTypeVisitor);
    auto* type = genTypeVisitor.getType();

    // Default value for initializing variable (0 for integers, 0.0 for reals)
    llvm::Constant* defaultV = getDefaultValueForType(type, gen.ctx);

    if (node.isGlobal()) {
        auto* gVar = new llvm::GlobalVariable(gen.module, type, false, llvm::GlobalValue::ExternalLinkage, defaultV,
                                              node.getDeclName());

        gen.symbolTable.addSymbol(node.getDeclName(), {node.getDeclName(), node.getTypeNode().get(), gVar, false});
    } else {
        // CreateAlloca creates an allocation instruction in the IR.
        // This instruction allocated memory on the stack for the variable.
        // store=memPtr is a pointer to the allocated memory.
        auto* memPtr = gen.builder.CreateAlloca(type, nullptr, node.getDeclName());
        gen.builder.CreateStore(defaultV, memPtr);

        gen.symbolTable.addSymbol(node.getDeclName(), {node.getDeclName(), node.getTypeNode().get(), memPtr, false});
    }

    value = nullptr;
}

void CodeGenVisitor::visit(ArrayDeclASTNode& node) {
    // Validations
    if (gen.symbolTable.contains(node.getDeclName()))
        throw CodeGenException("Array is already declared: " + node.getDeclName());

    if (node.getTypeNode()->getLowerBound() > node.getTypeNode()->getUpperBound())
        throw CodeGenException("Array lower bound is greater than upper bound: " + node.getDeclName());

    if (node.getTypeNode()->getUpperBound() - node.getTypeNode()->getLowerBound() > 1000)
        throw CodeGenException("Array size is too large: " + node.getDeclName());

    if (node.getTypeNode()->getUpperBound() == node.getTypeNode()->getLowerBound())
        throw CodeGenException("Array size should be at least 2: " + node.getDeclName());

    // Get array type
    GenTypeVisitor genTypeVisitor(gen);
    node.getTypeNode()->accept(genTypeVisitor);
    auto* arrayType = genTypeVisitor.getType();

    if (node.isGlobal()) {
        auto* gVar = new llvm::GlobalVariable(gen.module, arrayType, false, llvm::GlobalValue::ExternalLinkage,
                                              llvm::ConstantAggregateZero::get(arrayType), node.getDeclName());

        gen.symbolTable.addSymbol(node.getDeclName(), {node.getDeclName(), node.getTypeNode().get(), gVar, false});
    } else {
        llvm::AllocaInst* store = gen.builder.CreateAlloca(arrayType, nullptr, node.getDeclName());

        // Initialize array with default values (0 for integers, 0.0 for reals)
        llvm::Value* zeroIndex = llvm::ConstantInt::get(gen.ctx, llvm::APInt(32, 0, true));

        auto* elementType = arrayType->getArrayElementType();
        llvm::Constant* defaultV = getDefaultValueForType(elementType, gen.ctx);

        unsigned arraySize = node.getTypeNode()->getUpperBound() - node.getTypeNode()->getLowerBound() + 1;
        for (unsigned i = 0; i < arraySize; ++i) {
            llvm::Value* index = llvm::ConstantInt::get(gen.ctx, llvm::APInt(32, i, true));
            llvm::Value* ptrToElem = gen.builder.CreateGEP(arrayType, store, {zeroIndex, index});
            gen.builder.CreateStore(defaultV, ptrToElem);
        }

        gen.symbolTable.addSymbol(node.getDeclName(), {node.getDeclName(), node.getTypeNode().get(), store, false});
    }

    value = nullptr;
}

void CodeGenVisitor::visit(ConstDefASTNode& node) {
    if (gen.symbolTable.contains(node.getDeclName()))
        throw CodeGenException("Constant is already defined: " + node.getDeclName());

    node.getExprNode()->accept(*this);
    auto* exprV = value;

    if (!exprV)
        throw CodeGenException("Constant expression value is not found: " + node.getDeclName());

    // Create type node for the constant by inferring type of the expression value
    if (exprV->getType()->isDoubleTy())
        node.setTypeNode(std::make_unique<PrimitiveTypeASTNode>(PrimitiveTypeASTNode::PrimitiveType::REAL));
    else if (exprV->getType()->isIntegerTy())
        node.setTypeNode(std::make_unique<PrimitiveTypeASTNode>(PrimitiveTypeASTNode::PrimitiveType::INTEGER));
    else
        throw CodeGenException("Unsupported constant type: " + node.getDeclName());

    if (node.isGlobal()) {
        llvm::Constant* defaultV = getDefaultValueForType(exprV->getType(), gen.ctx);

        auto* gConst = new llvm::GlobalVariable(gen.module, defaultV->getType(), false,
                                                llvm::GlobalValue::ExternalLinkage, defaultV, node.getDeclName());

        gen.builder.CreateStore(exprV, gConst);

        gen.symbolTable.addSymbol(node.getDeclName(),
                                  {node.getDeclName(), node.getTypeNode().value().get(), gConst, true});
    } else {
        // Create an alloca instruction to store the constant in the symbol table
        auto* store = gen.builder.CreateAlloca(exprV->getType(), nullptr, node.getDeclName());

        // Store a value 'val' into a memory location 'store'
        gen.builder.CreateStore(exprV, store);

        // Add the constant symbol to the symbol table
        gen.symbolTable.addSymbol(node.getDeclName(),
                                  {node.getDeclName(), node.getTypeNode().value().get(), store, true});
    }

    value = nullptr;
}

void CodeGenVisitor::visit(ProcDeclASTNode& node) {
    // Check if this procedure has already been declared
    auto* proc = gen.module.getFunction(node.getDeclName());

    if (proc) {
        if (!node.getBlockNode().has_value())
            throw CodeGenException("Redeclaration of procedure '" + node.getDeclName() + "'");

        if (!proc->empty())
            throw CodeGenException("Redefinition of procedure '" + node.getDeclName() + "'");

        if (proc->arg_size() != node.getParamNodes().size())
            throw CodeGenException("Procedure '" + node.getDeclName() + "' expects " +
                                   std::to_string(proc->arg_size()) + " arguments in declaration, but " +
                                   std::to_string(node.getParamNodes().size()) + " were provided in definition");

        for (auto& arg : proc->args()) {
            GenTypeVisitor genTypeVisitor(gen);
            node.getParamNodes()[arg.getArgNo()]->getTypeNode()->accept(genTypeVisitor);
            auto* defArgType = genTypeVisitor.getType();

            if (arg.getType() != defArgType)
                throw CodeGenException("Procedure '" + node.getDeclName() + "' expects argument $" +
                                       std::to_string(arg.getArgNo()) + " to be of type provided in the declaration");
        }

    } else {
        // Procedure has not been declared yet, so declare it now

        // Get procedure's parameters types
        std::vector<llvm::Type*> paramTypes;
        for (const auto& p : node.getParamNodes()) {
            GenTypeVisitor genTypeVisitor(gen);
            p->getTypeNode()->accept(genTypeVisitor);
            paramTypes.push_back(genTypeVisitor.getType());
        }

        // Create the procedure
        llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), paramTypes, false);
        // ExternalLinkage means the function can be called from other modules, gen.module is the module where the function will be added.
        proc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.getDeclName(), gen.module);

        // If it's just a forward declaration (no body), return
        if (!node.getBlockNode().has_value()) {
            value = nullptr;
            return;
        }
    }

    // At this point, procedure is either a new declaration or a forward declaration needing a body

    // Save to restore it later
    auto* prevBB = gen.builder.GetInsertBlock();
    auto symbolTableCopy = gen.symbolTable;

    // Set up the entry block for that new procedure
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(gen.ctx, "entry", proc);
    gen.builder.SetInsertPoint(BB);

    // Allocate space for procedure parameters and copy argument values into that allocated space
    unsigned i = 0;
    for (auto& arg : proc->args()) {
        // Name the argument
        arg.setName(node.getParamNodes()[i]->getDeclName());

        // Create an entry in the procedure's stack frame for this argument and store the argument's value in it
        auto* store = gen.builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
        gen.builder.CreateStore(&arg, store);

        gen.symbolTable.addSymbol(arg.getName().str(),
                                  {arg.getName().str(), node.getParamNodes()[i]->getTypeNode().get(), store, false});
        ++i;
    }

    // Find exit statements and set the return value to void
    CollectorVisitor<ExitASTNode> exitNodesVisitor;
    node.accept(exitNodesVisitor);
    for (auto exitNode : exitNodesVisitor.collectedNodes) {
        exitNode->setRetV(std::monostate{});
    }

    // Generate code for the procedure's block
    node.getBlockNode().value()->accept(*this);

    // Set return void
    gen.builder.CreateRetVoid();

    //    llvm::verifyFunction(*proc, &llvm::errs()); // DEBUG

    // Restore the previous insertion point and symbol table
    gen.builder.SetInsertPoint(prevBB);
    gen.symbolTable = symbolTableCopy;

    value = nullptr;
}

void CodeGenVisitor::visit(FunDeclASTNode& node) {
    auto* func = gen.module.getFunction(node.getDeclName());

    // Validations and function declaration
    if (func) {
        if (!node.getBlockNode().has_value())
            throw CodeGenException("Redeclaration of function '" + node.getDeclName() + "'");

        if (!func->empty())
            throw CodeGenException("Redefinition of function '" + node.getDeclName() + "'");

        if (func->arg_size() != node.getParamNodes().size())
            throw CodeGenException("Function '" + node.getDeclName() + "' expects " + std::to_string(func->arg_size()) +
                                   " arguments in declaration, but " + std::to_string(node.getParamNodes().size()) +
                                   " were provided in definition");

        for (auto& arg : func->args()) {
            GenTypeVisitor genTypeVisitor(gen);
            node.getParamNodes()[arg.getArgNo()]->getTypeNode()->accept(genTypeVisitor);
            auto* defArgType = genTypeVisitor.getType();

            if (arg.getType() != defArgType)
                throw CodeGenException("Function '" + node.getDeclName() + "' expects argument $" +
                                       std::to_string(arg.getArgNo()) + " to be of type provided in the declaration");
        }
    } else {
        for (const auto& p : node.getParamNodes())
            if (p->getDeclName() == node.getDeclName())
                throw CodeGenException("Function parameter has the same name as the function itself: '" +
                                       node.getDeclName() + "'");

        std::vector<llvm::Type*> paramTypes;
        for (const auto& p : node.getParamNodes()) {
            GenTypeVisitor genTypeVisitor(gen);
            p->getTypeNode()->accept(genTypeVisitor);
            paramTypes.push_back(genTypeVisitor.getType());
        }

        GenTypeVisitor genTypeVisitor(gen);
        node.getRetTypeNode()->accept(genTypeVisitor);
        auto* retType = genTypeVisitor.getType();

        llvm::FunctionType* funcType = llvm::FunctionType::get(retType, paramTypes, false);
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.getDeclName(), gen.module);

        if (!node.getBlockNode().has_value()) {
            value = nullptr;
            return;
        }
    }

    // At this point, function is either a new declaration or a forward declaration needing a body, so proceed to the body
    auto prevBB = gen.builder.GetInsertBlock();
    auto symbolTableCopy = gen.symbolTable;

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(gen.ctx, "entry", func);
    gen.builder.SetInsertPoint(BB);

    unsigned i = 0;
    for (auto& arg : func->args()) {
        arg.setName(node.getParamNodes()[i]->getDeclName());
        auto* store = gen.builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
        gen.builder.CreateStore(&arg, store);
        gen.symbolTable.addSymbol(arg.getName().str(),
                                  {arg.getName().str(), node.getParamNodes()[i]->getTypeNode().get(), store, false});
        ++i;
    }

    // Get function's return type
    GenTypeVisitor genTypeVisitor(gen);
    node.getRetTypeNode()->accept(genTypeVisitor);
    auto* retType = genTypeVisitor.getType();

    // Create a variable that represents the function's return value. It has the name of the function
    auto* retValStore = gen.builder.CreateAlloca(retType, nullptr, node.getDeclName());
    llvm::Constant* defaultV = getDefaultValueForType(retType, gen.ctx);
    gen.builder.CreateStore(defaultV, retValStore);
    gen.symbolTable.addSymbol(node.getDeclName(),
                              {node.getDeclName(), node.getRetTypeNode().get(), retValStore, false});

    // Load the return value from the variable that represents the function's return value
    auto loadReturnV = [&]() {
        auto* loadedReturnV = gen.builder.CreateLoad(retType, retValStore, node.getDeclName());
        return loadedReturnV;
    };

    // Find exit statements and set the return value
    CollectorVisitor<ExitASTNode> exitNodesVisitor;
    node.getBlockNode().value()->accept(exitNodesVisitor);
    for (auto exitNode : exitNodesVisitor.collectedNodes) {
        exitNode->setRetV(loadReturnV);
    }

    // Emit body code
    node.getBlockNode().value()->accept(*this);

    gen.builder.CreateRet(loadReturnV());

    //    llvm::verifyFunction(*func, &llvm::errs()); // DEBUG

    gen.builder.SetInsertPoint(prevBB);
    gen.symbolTable = symbolTableCopy;

    value = nullptr;
}

void CodeGenVisitor::visit(AssignASTNode& node) {
    if (!gen.symbolTable.contains(node.getVarNode()->getRefName()))
        throw CodeGenException("Variable not found: " + node.getVarNode()->getRefName());

    if (gen.symbolTable.getSymbol(node.getVarNode()->getRefName()).immutable)
        throw CodeGenException("Cannot assign to a constant: " + node.getVarNode()->getRefName());

    // Get memory location of the variable
    StoreVisitor storeVisitor(gen);
    node.getVarNode()->accept(storeVisitor);
    auto* store = storeVisitor.getStore();

    node.getExprNode()->accept(*this);
    auto* exprVal = value;

    if (!exprVal)
        throw CodeGenException("Assignment failed - expression value is not found");

    if (exprVal->getType()->isDoubleTy() && store->getType()->getPointerElementType()->isIntegerTy())
        throw CodeGenException("Assignment failed - cannot assign real value to an integer variable: " +
                               node.getVarNode()->getRefName());

    // Handle implicit conversion int -> double
    if (store->getType()->getPointerElementType()->isDoubleTy())
        exprVal = gen.builder.CreateSIToFP(exprVal, llvm::Type::getDoubleTy(gen.ctx));

    gen.builder.CreateStore(exprVal, store);

    value = nullptr;
}

void CodeGenVisitor::visit(IfASTNode& node) {
    auto* func = gen.builder.GetInsertBlock()->getParent();

    if (!func)
        throw CodeGenException("Parent function is not found");

    llvm::BasicBlock* BBbody = llvm::BasicBlock::Create(gen.ctx, "body", func);
    llvm::BasicBlock* BBelseBody = llvm::BasicBlock::Create(gen.ctx, "elseBody", func);
    llvm::BasicBlock* BBafter = llvm::BasicBlock::Create(gen.ctx, "after", func);

    node.getCondNode()->accept(*this);
    auto* condVal = value;
    gen.builder.CreateCondBr(condVal, BBbody, BBelseBody);

    gen.builder.SetInsertPoint(BBbody);
    node.getBodyNode()->accept(*this);
    gen.builder.CreateBr(BBafter);

    gen.builder.SetInsertPoint(BBelseBody);
    if (node.getElseBodyNode().has_value()) {
        node.getElseBodyNode().value()->accept(*this);
    }
    gen.builder.CreateBr(BBafter);

    gen.builder.SetInsertPoint(BBafter);

    value = nullptr;
}

void CodeGenVisitor::visit(WhileASTNode& node) {
    auto* func = gen.builder.GetInsertBlock()->getParent();

    if (!func)
        throw CodeGenException("Parent function is not found");

    llvm::BasicBlock* BBcond = llvm::BasicBlock::Create(gen.ctx, "cond", func);
    llvm::BasicBlock* BBbody = llvm::BasicBlock::Create(gen.ctx, "body", func);
    llvm::BasicBlock* BBafter = llvm::BasicBlock::Create(gen.ctx, "after", func);

    gen.builder.CreateBr(BBcond);

    gen.builder.SetInsertPoint(BBcond);
    node.getCondNode()->accept(*this);
    auto* condVal = value;
    gen.builder.CreateCondBr(condVal, BBbody, BBafter);

    gen.builder.SetInsertPoint(BBbody);

    // Find break statements and set their break block
    CollectorVisitor<BreakASTNode> breakNodesVisitor;
    node.getBodyNode()->accept(breakNodesVisitor);
    for (auto breakNode : breakNodesVisitor.collectedNodes)
        breakNode->setBreakBlock(BBafter);

    node.getBodyNode()->accept(*this);

    gen.builder.CreateBr(BBcond);

    gen.builder.SetInsertPoint(BBafter);

    value = nullptr;
}

void CodeGenVisitor::visit(ForASTNode& node) {
    auto* func = gen.builder.GetInsertBlock()->getParent();

    if (!func)
        throw CodeGenException("Parent function is not found");

    llvm::BasicBlock* BBinit = llvm::BasicBlock::Create(gen.ctx, "init", func);
    llvm::BasicBlock* BBcond = llvm::BasicBlock::Create(gen.ctx, "cond", func);
    llvm::BasicBlock* BBbody = llvm::BasicBlock::Create(gen.ctx, "body", func);
    llvm::BasicBlock* BBafter = llvm::BasicBlock::Create(gen.ctx, "after", func);

    // Branch to the initialization block
    gen.builder.CreateBr(BBinit);
    // Emit code for the initialization block
    gen.builder.SetInsertPoint(BBinit);
    node.getInitNode()->accept(*this);
    gen.builder.CreateBr(BBcond);

    // Emit code for the condition block
    gen.builder.SetInsertPoint(BBcond);
    // Get memory location of the variable
    StoreVisitor storeVisitor(gen);
    node.getInitNode()->getVarNode()->accept(storeVisitor);
    auto* fromStore = storeVisitor.getStore();
    auto fromVal = [&]() {
        node.getInitNode()->getVarNode()->accept(*this);
        return value;
    };
    node.getToNode()->accept(*this);
    auto* toVal = value;
    auto* condVal = (node.isIncreasing()) ? gen.builder.CreateICmpSLE(fromVal(), toVal, "le")
                                          : gen.builder.CreateICmpSGE(fromVal(), toVal, "ge");
    gen.builder.CreateCondBr(condVal, BBbody, BBafter);

    // Generate code for the body block
    gen.builder.SetInsertPoint(BBbody);
    // Find break statements and set their break block
    CollectorVisitor<BreakASTNode> breakNodesVisitor;
    node.getBodyNode()->accept(breakNodesVisitor);
    for (auto breakNode : breakNodesVisitor.collectedNodes)
        breakNode->setBreakBlock(BBafter);

    node.getBodyNode()->accept(*this);

    auto* incrementedVal = gen.builder.CreateAdd(fromVal(), gen.builder.getInt32(node.isIncreasing() ? 1 : -1));
    gen.builder.CreateStore(incrementedVal, fromStore);
    gen.builder.CreateBr(BBcond);

    gen.builder.SetInsertPoint(BBafter);

    value = nullptr;
}

void CodeGenVisitor::visit(ProcCallASTNode& node) {
    gen.funcHandler->handle(node.getProcName(), node.getArgNodes());
}

void CodeGenVisitor::visit([[maybe_unused]] EmptyStmtASTNode& node) {
    value = nullptr;
}

void CodeGenVisitor::visit(ProgramASTNode& node) {
    node.getBlockNode()->setMain(true);
    node.getBlockNode()->accept(*this);
    value = nullptr;
}

void CodeGenVisitor::visit(BreakASTNode& node) {
    if (!node.getBreakBlock()) {
        value = nullptr;
        return;
    }

    gen.builder.CreateBr(node.getBreakBlock());

    auto* func = gen.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* afterBreakBlock = llvm::BasicBlock::Create(gen.ctx, "afterBreak", func);
    gen.builder.SetInsertPoint(afterBreakBlock);

    value = nullptr;
}

void CodeGenVisitor::visit(ExitASTNode& node) {
    if (std::holds_alternative<std::monostate>(node.getRetV()))
        gen.builder.CreateRetVoid();
    else if (std::holds_alternative<llvm::Value*>(node.getRetV()))
        gen.builder.CreateRet(std::get<llvm::Value*>(node.getRetV()));
    else
        gen.builder.CreateRet(std::get<std::function<llvm::LoadInst*()>>(
            node.getRetV())());  // calls load instruction which returns ret value

    // Any emitted LLVM IR code after the exit statement is unreachable and will be removed by the optimizer.
    auto* func = gen.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* afterExitBlock = llvm::BasicBlock::Create(gen.ctx, "afterExit", func);
    gen.builder.SetInsertPoint(afterExitBlock);

    value = nullptr;
}
