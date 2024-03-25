#pragma once
#include "AST.hpp"
#include "FuncHandler.hpp"

struct Symbol {
    std::string name;
    /**
     * @brief The type of the symbol
     */
    TypeASTNode* type;

    /**
     * @brief The memory location of the symbol
     * @note If the symbol is a global variable, the memory location is a GlobalVariable.
     */
    std::variant<llvm::AllocaInst*, llvm::GlobalVariable*> memPtr;

    bool immutable;

    [[nodiscard]] bool isGlobal() const;
    [[nodiscard]] llvm::GlobalVariable* getGlobalMemPtr() const;
    [[nodiscard]] llvm::AllocaInst* getLocalMemPtr() const;
};

class SymbolTable {
   private:
    std::map<std::string, Symbol> table;

   public:
    /**
     * @brief Adds a symbol to the table
     */
    void addSymbol(const std::string& name, const Symbol& symbol);

    /**
     * @brief Returns the symbol with the given name
     */
    [[nodiscard]] const Symbol& getSymbol(const std::string& name) const;

    /**
     * @brief Checks if the symbol is in the table
     */
    [[nodiscard]] bool contains(const std::string& name) const;
};

/**
 * @brief LLVM context for code generation
 */
struct GenContext {
    /**
     * @brief Environment/Context for the code generation
     * @note Holds the global state required during the LLVM IR construction process, including unique instances of types, constant values, metadata
     */
    llvm::LLVMContext ctx;

    /**
     * @brief Simplifies the generation of LLVM IR code with abstractions
     */
    llvm::IRBuilder<> builder;

    /**
     * @brief Container for functions, global variables, etc.
     */
    llvm::Module module;

    SymbolTable symbolTable;

    /**
     * @brief Chain of responsibility for function/procedure calls both predefined and user-defined
     */
    std::shared_ptr<FuncHandler> funcHandler;

    explicit GenContext(const std::string& moduleName);
};

class CodeGenerator {
   private:
    ASTNode* astNode;

   public:
    explicit CodeGenerator(ASTNode* astNode);

    /**
     * @brief Generates and dumps to standard output the LLVM IR code for 'astNode'
     */
    void generate() const;

    /**
     * @brief Generates and dumps to file 'outFile' the LLVM IR code for 'astNode'
     */
    void generate(const std::string& outFile) const;
};

class CodeGenException : public std::exception {
   private:
    std::string message;

   public:
    explicit CodeGenException(std::string msg);

    [[nodiscard]] const char* what() const noexcept override;
};
