#include "SimpleConsoleView.hpp"
#include "ast/CodeGenerator.hpp"
#include "ast/visitor/PrintVisitor.hpp"
#include "lexer/Lexer.hpp"
#include "parser/Parser.hpp"

void SimpleConsoleView::showHelp() const {
    std::cout << "Usage: milac [options] source.mila\n"
              << "Options:\n"
              << "  --help          Show this help message\n"
              << "  -v              Enable verbose debugging\n"
              << "  -o <file>       Specify output executable file name\n";
}

int SimpleConsoleView::run(const std::vector<std::string>& args) {
    if (parseArgs(args) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if (!readyToRun)
        return EXIT_SUCCESS;

    std::ifstream file(inputFileName);
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::istringstream input(fileContent);

    Lexer lexer(input);
    Parser parser(lexer, verbose);

    if (verbose) {
        std::cout << "---------- LEXER -------------------\n";
    }

    std::unique_ptr<ProgramASTNode> programNode;

    try {
        programNode = parser.parseProgram();
    } catch (const ParserException& e) {
        std::cerr << "Parser error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const LexerException& e) {
        std::cerr << "Lexer error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (verbose) {
        std::cout << "---------- PARSED AST --------------\n";
        PrintVisitor printVisitor(std::cout);
        programNode->accept(printVisitor);
    }

    CodeGenerator codegen(programNode.get());

    try {
        codegen.generate("output.ir");
    } catch (const CodeGenException& e) {
        std::cerr << "Code generation error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    Utils::ProgramRunResult llcResult = Utils::exec(R"(llc "output.ir" -o "output.s" -relocation-model=pic)");

    if (llcResult.exitCode != 0) {
        std::cerr << "LLVM IR to assembly compilation failed with exit code " << llcResult.exitCode << std::endl;
        return EXIT_FAILURE;
    }

    std::string ioCodeInC =
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "int write_int(int x) { printf(\"%d\", x); return 0;}int write_double(double x) { printf(\"%.3f\", x); return "
        "0;}int writeln_int(int x) { printf(\"%d\\n\", x); return 0;}int writeln_double(double x) { "
        "printf(\"%.3f\\n\", x); return 0;}int readln_int(int *x) { scanf(\"%d\", x); return 0;}int "
        "readln_double(double *x) { scanf(\"%lf\", x); return 0;} int error(char *s) { printf(\"%s\", s); exit(1); }";

    std::ofstream ioFile("io.c");
    if (!ioFile.is_open()) {
        std::cerr << "Error: failed to open io.c file for writing" << std::endl;
        return EXIT_FAILURE;
    }
    ioFile << ioCodeInC;
    ioFile.close();

    std::string clangCmd = "clang output.s io.c -o " + outputFileName;
    Utils::ProgramRunResult clangResult = Utils::exec(clangCmd);

    if (clangResult.exitCode != 0) {
        std::cerr << "Assembly to object compilation failed with exit code " << clangResult.exitCode << std::endl;
        return EXIT_FAILURE;
    }

    // clear all generated files - uncomment if you want to keep them
    std::remove("output.ir");
    std::remove("output.s");
    std::remove("io.c");

    return EXIT_SUCCESS;
}

int SimpleConsoleView::parseArgs(const std::vector<std::string>& args) {
    if (args.empty()) {
        showHelp();
        return EXIT_FAILURE;
    }

    if (args.size() == 1 && args[0] == "--help") {
        showHelp();
        return EXIT_SUCCESS;
    }

    for (unsigned i = 0; i < args.size(); ++i) {
        if (args[i] == "-v") {
            verbose = true;
        } else if (args[i] == "-o") {
            if (i + 1 < args.size()) {
                outputFileName = (args[i + 1] + ".out");
                ++i;
            } else {
                std::cerr << "Error: missing output filename" << std::endl;
                return EXIT_FAILURE;
            }
        } else if (Utils::hasExtension(args[i], ".mila")) {
            inputFileName = args[i];
        } else {
            std::cerr << "Error: unknown option or invalid file: " << args[i] << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (inputFileName.empty()) {
        std::cerr << "Error: missing input file" << std::endl;
        return EXIT_FAILURE;
    }

    if (outputFileName.empty()) {
        outputFileName = "a.out";
    }

    readyToRun = true;

    return EXIT_SUCCESS;
}
