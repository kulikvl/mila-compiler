find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_library(mila_lib STATIC
        lexer/Lexer.hpp
        lexer/Lexer.cpp
        lexer/Token.hpp
        lexer/Token.cpp
        lexer/Position.hpp
        lexer/Position.cpp
        parser/Parser.hpp
        parser/Parser.cpp
        ast/AST.hpp
        ast/AST.cpp
        ast/visitor/ASTNodeVisitor.hpp
        ast/visitor/CollectorVisitor.hpp
        ast/visitor/PrintVisitor.cpp
        ast/visitor/PrintVisitor.hpp
        ast/visitor/DefaultVisitor.cpp
        ast/visitor/DefaultVisitor.hpp
        ast/visitor/GenTypeVisitor.cpp
        ast/visitor/GenTypeVisitor.hpp
        ast/visitor/CodeGenVisitor.cpp
        ast/visitor/CodeGenVisitor.hpp
        ast/CodeGenerator.cpp
        ast/CodeGenerator.hpp
        ast/visitor/StoreVisitor.cpp
        ast/visitor/StoreVisitor.hpp
        ast/FuncHandler.cpp
        ast/FuncHandler.hpp
        utils/Utils.cpp
        utils/Utils.hpp
        ui/SimpleConsoleView.cpp
        ui/SimpleConsoleView.hpp
)

target_include_directories(mila_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# 'SYSTEM' to suppress warnings from llvm headers
target_include_directories(mila_lib SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_options(mila_lib PRIVATE ${LLVM_DEFINITIONS_LIST})
llvm_config(mila_lib USE_SHARED support core irreader)

add_executable(mila main.cpp)
target_link_libraries(mila PRIVATE mila_lib)
