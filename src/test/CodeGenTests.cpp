#include <gtest/gtest.h>
#include <sstream>
#include <vector>
#include "ast/AST.hpp"
#include "ast/CodeGenerator.hpp"
#include "parser/Parser.hpp"
#include "utils/Utils.hpp"

void TestProgram(const std::string& src, const std::optional<std::string>& optInput, const int expectedExitCode,
                 const std::string& expectedOutput, bool debug = false) {
    std::istringstream input(src);
    Lexer lexer(input);
    Parser parser(lexer, debug);

    std::unique_ptr<ASTNode> programNode = parser.parseProgram();

    CodeGenerator codeGenerator(programNode.get());
    codeGenerator.generate("output.ir");

    Utils::ProgramRunResult llcResult = Utils::exec(R"(llc "output.ir" -o "output.s" -relocation-model=pic)");
    ASSERT_EQ(0, llcResult.exitCode);

    Utils::ProgramRunResult clangResult = Utils::exec(R"(clang "output.s" "io.c" -o "a.out")");
    ASSERT_EQ(0, clangResult.exitCode);

    std::string runCmd = "./a.out";

    if (optInput) {
        std::string echoCmd = "echo " + *optInput + " > input.txt";
        Utils::ProgramRunResult echoResult = Utils::exec(echoCmd);
        ASSERT_EQ(0, echoResult.exitCode);
        runCmd += " < input.txt";
    }

    Utils::ProgramRunResult programResult = Utils::exec(runCmd);

    if (debug) {
        std::cout << "---------- EXIT CODE ---------------\n" << programResult.exitCode << "\n";
        std::cout << "---------- PROGRAM OUTPUT ----------\n" << programResult.output << std::endl;
    }

    const int actualExitCode = programResult.exitCode;
    const std::string& actualOutput = programResult.output;

    EXPECT_EQ(expectedExitCode, actualExitCode);
    EXPECT_EQ(expectedOutput, actualOutput);
}

/* ================== IO Tests ================== */

TEST(CodeGenTests, HandlesReadln) {
    const std::string src =
        "program test;\n"
        "var n: integer;\n"
        "begin\n"
        " readln(n);\n"
        " write(n + 1);\n"
        "end.\n";
    const std::string input = "16";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "17";

    TestProgram(src, input, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesWriteln) {
    const std::string src =
        "program test;\n"
        "var n: integer;\n"
        "begin\n"
        " readln(n);\n"
        " writeln(n - 1);\n"
        "end.\n";
    const std::string input = "16";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "15\n";

    TestProgram(src, input, expectedExitCode, expectedOutput);
}

/* ================== Variables Tests ================== */

TEST(CodeGenTests, HandlesVariableDeclaration_1) {
    const std::string src =
        "program test;"
        "var x, y : integer;"
        "var z, w : real;"
        "begin"
        " x := 1; y := x + 1; z := y + 2;"
        " write(x); write(y); write(z); write(w);"
        "end.";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "124.0000.000";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesVariableDeclaration_2) {
    const std::string src =
        "program test;\n"
        "var x, y : integer;\n"
        "var z, w : integer;\n"
        "begin\n"
        " x := 1;\n"
        " y := x + 1;\n"
        " z := y * 2;\n"
        " w := z - 1;\n"
        " writeln(x);\n"
        " writeln(y);\n"
        " writeln(z);\n"
        " writeln(w);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "1\n2\n4\n3\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesGlobalVariables) {
    const std::string src =
        "program test;\n"
        "const K = 1 + 2;\n"
        "var y : integer;\n"
        "var arr : array [0 .. 10] of integer;\n"
        "procedure proc(); begin arr[5] := 1; end;\n"
        "function func(): integer; begin y := 3; end;\n"
        "begin\n"
        " arr[6] := 2;\n"
        " proc();\n"
        " func();\n"
        " write(arr[5]); write(arr[6]); write(K);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "123";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, ThrowsOnVariableConflict) {
    const std::string src =
        "program test;\n"
        "var y : integer;\n"
        "procedure proc(); const y = 5; begin end;\n"
        "begin\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnAccessingLocalVariableFromOuterScope) {
    const std::string src =
        "program test;\n"
        "procedure proc(); const y = 5; begin end;\n"
        "begin\n"
        " write(y);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

/* ================== Constants Tests ================== */

TEST(CodeGenTests, HandlesConstantDefinition) {
    const std::string src =
        "program test;\n"
        "const x = 1; y = 2;\n"
        "const z = 3; w = 4;\n"
        "begin\n"
        " writeln(x);\n"
        " writeln(y);\n"
        " writeln(z);\n"
        " writeln(w);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "1\n2\n3\n4\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, ThrowsOnAssignToGlobalConst) {
    const std::string src =
        "program test;\n"
        "const x = 10;\n"
        "begin\n"
        " x := 15;\n"
        " write(x);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnAssignToLocalConst) {
    const std::string src =
        "program test;\n"
        "procedure P(); const x = 10; begin x := 15; end;\n"
        "begin\n"
        " P();\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, HandlesConstantDefinitionWithExpression) {
    const std::string src =
        "program test;\n"
        "const x = 1 + 10 mod 3;\n"
        "const y = x + 1;\n"
        "procedure P(); const z = x + 2; w = z + y; begin write(z); write(w); end;"
        "begin\n"
        " write(x); write(y); P();\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "2347";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

/* ================== Array Tests ================== */

TEST(CodeGenTests, HandlesArrayOfIntegers) {
    const std::string src =
        "program test;\n"
        "var X : array [-50 .. 50] of integer;\n"
        "begin\n"
        " X[-50] := 13;\n"
        " X[50] := 14;\n"
        " X[0] := 15;\n"
        " X[1] := X[0] + X[50];\n"
        " writeln(X[0]);\n"
        " writeln(X[1]);\n"
        " writeln(X[50]);\n"
        " writeln(X[-50]);\n"
        " writeln(X[13]);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "15\n29\n14\n13\n0\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesArrayOfReals) {
    const std::string src =
        "program test;\n"
        "var X : array [-1 .. 3] of real;\n"
        "begin\n"
        " X[-1] := 0.03;\n"
        " X[3] := 0.08;\n"
        " X[1] := 0;\n"
        " X[1] := X[-1] + X[0] + X[3];\n"
        " writeln(X[1]);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "0.110\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, ThrowsOnRealArrayIndex) {
    const std::string src =
        "program test;\n"
        "var X : array [0 .. 5] of integer;\n"
        "begin\n"
        " X[1.5] := 1;\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnTooBigArraySize) {
    const std::string src =
        "program test;\n"
        "var X : array [0 .. 2000] of integer;\n"
        "begin\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnTooSmallArraySize) {
    const std::string src =
        "program test;\n"
        "var X : array [0 .. 0] of integer;\n"
        "begin\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnArrayReferenceOutOfBounds) {
    const std::string src =
        "program test;\n"
        "var X : array [-50 .. 50] of real;\n"
        "begin\n"
        " write(X[-51])\n"
        "end.\n";

    const int expectedExitCode = 1;
    const std::string expectedOutput = "Runtime error: Array 'X' - the index is out of bounds.\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, ThrowsOnArrayAssignOutOfBounds) {
    const std::string src =
        "program test;\n"
        "var X : array [-50 .. 50] of real;\n"
        "begin\n"
        " X[51] := 13;\n"
        "end.\n";

    const int expectedExitCode = 1;
    const std::string expectedOutput = "Runtime error: Array 'X' - the index is out of bounds.\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesArrayExpressionIndex) {
    const std::string src =
        "program test;\n"
        "var i : integer;\n"
        "var X : array [-50 .. 50] of integer;\n"
        "begin\n"
        " i := 1;\n"
        " X[i * 2 * 2] := 3;\n"
        " write(X[i * 4]);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "3";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

/* ================== Procedure Tests ================== */

TEST(CodeGenTests, HandlesProcedureDefinition) {
    const std::string src =
        "program test;\n"
        "procedure x();\n"
        "begin\n"
        " writeln(1)\n"
        "end;\n"
        "procedure y(x: integer);\n"
        "begin\n"
        " writeln(x)\n"
        "end;\n"
        "begin\n"
        " x();\n"
        " y(2)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "1\n2\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProcedureForwardDeclaration) {
    const std::string src =
        "program test;\n"
        "procedure x(); forward;\n"
        "procedure x();\n"
        "begin\n"
        " writeln(1)\n"
        "end;\n"
        "begin\n"
        " x();\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "1\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesExitFromProcedure) {
    const std::string src =
        "program test;\n"
        "procedure x();\n"
        "begin\n"
        " write(1);\n"
        " exit;\n"
        " write(2);\n"
        "end;\n"
        "begin\n"
        " write(3);\n"
        " x();\n"
        " exit;\n"
        " write(4);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "31";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, ThrowsOnBadProcedureDefinition_1) {
    const std::string src =
        "program test;\n"
        "procedure x(); forward;\n"
        "procedure x(a: integer);\n"
        "begin\n"
        " writeln(a)\n"
        "end;\n"
        "begin\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnBadProcedureDefinition_2) {
    const std::string src =
        "program test;\n"
        "procedure x(a: integer); forward;\n"
        "procedure x(a: real);\n"
        "begin\n"
        " writeln(a)\n"
        "end;\n"
        "begin\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnProcedureRedeclaration_1) {
    const std::string src =
        "program test;\n"
        "procedure x(); forward;\n"
        "procedure x(); forward;\n"
        "begin\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnProcedureRedeclaration_2) {
    const std::string src =
        "program test;\n"
        "procedure x(); begin end;\n"
        "procedure x(); forward;\n"
        "begin\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnProcedureRedefinition) {
    const std::string src =
        "program test;\n"
        "procedure x(); begin end;\n"
        "procedure x(); begin end;\n"
        "begin\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, ThrowsOnUnknownProcedureCall) {
    const std::string src =
        "program test;\n"
        "procedure x();\n"
        "begin\n"
        " writeln(1)\n"
        "end;\n"
        "begin\n"
        " y(2)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, HandlesProcedureRecursion) {
    const std::string src =
        "program test;\n"
        "procedure f(n: integer); forward;\n"
        "procedure g(n: integer);\n"
        " begin\n"
        "  if (n <= 0) then exit;\n"
        "  write(2); f(n - 1);\n"
        " end;\n"
        "procedure f(n: integer);\n"
        " begin\n"
        "  if (n <= 0) then exit;\n"
        "  write(1); g(n - 1);\n"
        " end;\n"
        "begin\n"
        " f(6);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "121212";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

/* ================== Function Tests ================== */

TEST(CodeGenTests, HandlesFunctionDefinition) {
    const std::string src =
        "program test;\n"
        "function add(a: integer; b: integer): integer;\n"
        "begin\n"
        " add := a + b\n"
        "end;\n"
        "var result: integer;\n"
        "begin\n"
        " result := add(1, 2);\n"
        " write(result);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "3";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesFunctionDefinitionWithVariables) {
    const std::string src =
        "program test;\n"
        " const G = 9;\n"
        "function add(a: integer; b: integer): integer;\n"
        " const L = G * 2;\n"
        " begin\n"
        "  add := a + b + L + G + add;\n"
        " end;\n"
        "begin\n"
        " write(add(1, 2));\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "30";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesExitFromFunction) {
    const std::string src =
        "program test;\n"
        "function multiply(a: integer; b: integer): integer;\n"
        "var factor : integer;\n"
        "begin\n"
        " multiply := a * b;\n"
        " break;\n"
        " exit;\n"
        " write(9);\n"
        " multiply := 0;\n"
        "end;\n"
        "begin\n"
        " write(multiply(2, 3));\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "6";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, ThrowsOnBadFunctionParameterName) {
    const std::string src =
        "program test;\n"
        "function x(x: integer): integer;\n"
        "begin end;\n"
        "begin\n"
        " write(x(1));\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

TEST(CodeGenTests, HandlesVariableSameNameAsFunction) {
    const std::string src =
        "program test;\n"
        "procedure proc(); begin write(1) end;"
        "function func(): integer; begin func := 4; write(9) end;\n"
        "var proc, func: integer;\n"
        "begin\n"
        " proc();\n"
        " func := func();"
        " write(func);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "194";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

/* ================== If-Then-Else Tests ================== */

TEST(CodeGenTests, HandlesIfStatement_1) {
    const std::string src =
        "program test;\n"
        "const x = 150;\n"
        "begin\n"
        " if x > 100 then write(1);\n"
        " write(2);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "12";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesIfStatement_2) {
    const std::string src =
        "program test;\n"
        "const x = 50;\n"
        "begin\n"
        " if x > 100 then write(1);\n"
        " write(2);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "2";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesIfElseStatement_1) {
    const std::string src =
        "program test;\n"
        "const x = 150;\n"
        "begin\n"
        " if x > 100 then write(1) else write(2);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "1";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesIfElseStatement_2) {
    const std::string src =
        "program test;\n"
        "const x = 50;\n"
        "begin\n"
        " if x > 100 then write(1) else write(2);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "2";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesNestedIfElseStatement_1) {
    const std::string src =
        "program test;\n"
        "const x = 150;\n"
        "begin\n"
        " if x > 100 then\n"
        "  if x < 200 then\n"
        "   write(1)\n"
        "  else\n"
        "   write(2)\n"
        " else\n"
        "  if x > 50 then\n"
        "   write(3)\n"
        "  else\n"
        "   write(4)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "1";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesNestedIfElseStatement_2) {
    const std::string src =
        "program test;\n"
        "const x = 250;\n"
        "begin\n"
        " if x > 100 then\n"
        "  if x < 200 then\n"
        "   write(1)\n"
        "  else\n"
        "   write(2)\n"
        " else\n"
        "  if x > 50 then\n"
        "   write(3)\n"
        "  else\n"
        "   write(4)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "2";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesNestedIfElseStatement_3) {
    const std::string src =
        "program test;\n"
        "const x = 75;\n"
        "begin\n"
        " if x > 100 then\n"
        "  if x < 200 then\n"
        "   write(1)\n"
        "  else\n"
        "   write(2)\n"
        " else\n"
        "  if x > 50 then\n"
        "   write(3)\n"
        "  else\n"
        "   write(4)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "3";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesNestedIfElseStatement_4) {
    const std::string src =
        "program test;\n"
        "const x = 50;\n"
        "begin\n"
        " if x > 100 then\n"
        "  if x < 200 then\n"
        "   write(1)\n"
        "  else\n"
        "   write(2)\n"
        " else\n"
        "  if x > 50 then\n"
        "   write(3)\n"
        "  else\n"
        "   write(4)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "4";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesDanglingElse_1) {
    const std::string src =
        "program test;\n"
        "const x = 50;\n"
        "begin\n"
        " if x > 100 then\n"
        "  if x < 200 then\n"
        "   write(1)\n"
        "  else\n"
        "   write(2)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesDanglingElse_2) {
    const std::string src =
        "program test;\n"
        "const x = 300;\n"
        "begin\n"
        " if x > 100 then\n"
        "  if x < 200 then\n"
        "   write(1)\n"
        "  else\n"
        "   write(2)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "2";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

/* ================== While Tests ================== */

TEST(CodeGenTests, HandlesWhileLoop) {
    const std::string src =
        "program test;\n"
        "var X : integer;\n"
        "begin\n"
        " X := 0;\n"
        " while X < 5 do\n"
        "  begin\n"
        "   write(X);\n"
        "   X := X + 1;\n"
        "  end\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "01234";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesWhileLoopWithBreak) {
    const std::string src =
        "program test;\n"
        "var X : integer;\n"
        "begin\n"
        " X := 0;\n"
        " while X < 5 do\n"
        "  begin\n"
        "   write(X);\n"
        "   if X = 3 then break;\n"
        "   X := X + 1;\n"
        "  end\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "0123";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesNestedWhileLoopWithBreak) {
    const std::string src =
        "program test;\n"
        "var X, Y : integer;\n"
        "begin\n"
        " X := 0;\n"
        " while X < 10 do\n"
        "  begin\n"
        "   while X < 5 do\n"
        "    begin\n"
        "     if X >= 2 then break;\n"
        "     Y := Y + 10;\n"
        "     X := X + 1;\n"
        "    end;\n"
        "   Y := Y + 100;\n"
        "   if X = 4 then break;\n"
        "   X := X + 1;\n"
        "  end;\n"
        " write(Y)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "320";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

/* ================== For Tests ================== */

TEST(CodeGenTests, HandlesForLoop) {
    const std::string src =
        "program test;\n"
        "var I : integer;\n"
        "begin\n"
        " for I := 0 to 5 do\n"
        "  write(I)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "012345";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesForDowntoLoop) {
    const std::string src =
        "program test;\n"
        "var I : integer;\n"
        "begin\n"
        " for I := 5 downto 0 do\n"
        "  write(I)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "543210";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesForLoopWithBreak_1) {
    const std::string src =
        "program test;\n"
        "var I : integer;\n"
        "begin\n"
        " for I := 0 to 5 do\n"
        "  begin\n"
        "   write(I);\n"
        "   if I = 3 then break;\n"
        "  end;\n"
        " write(9)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "01239";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesForLoopWithBreak_2) {
    const std::string src =
        "program test;\n"
        "var I : integer;\n"
        "begin\n"
        " for I := 0 to 5 do\n"
        "  begin\n"
        "   write(1);\n"
        "   break;break;break;\n"
        "   write(2);\n"
        "  end;\n"
        " break;\n"
        " write(3)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "13";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesNestedForLoopWithBreak) {
    const std::string src =
        "program test;\n"
        "var I, J : integer;\n"
        "begin\n"
        " for I := 1 to 3 do\n"
        "  for J := 0 to 5 do\n"
        "   begin\n"
        "    write(I + J);\n"
        "    if J = 1 then break;\n"
        "   end;\n"
        " write(9)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "1223349";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesDummyForLoop) {
    const std::string src =
        "program test;\n"
        "var I : integer;\n"
        "begin\n"
        " for I := 10 to -5 do\n"
        "  write(I)\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

/* ================== Expression Tests ================== */

TEST(CodeGenTests, HandlesOperatorPrecedence) {
    const std::string src =
        "program test;\n"
        "begin\n"
        " writeln(2 * 3 - 1);\n"
        " writeln(2 - 3 * 1);\n"
        " writeln(2 * 3 + 1);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "5\n-1\n7\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesUnaryExpressions) {
    const std::string src =
        "program test;\n"
        "begin\n"
        " writeln(-2 * -3 + -1);\n"
        " writeln(---1);\n"
        " writeln(not 1);\n"
        " if not (1 = 2) then writeln(1);\n"
        " if not (1 = 1) then writeln(2);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "5\n-1\n0\n1\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesOperatorAssociativity) {
    const std::string src =
        "program test;\n"
        "var x : integer;\n"
        "begin\n"
        " x := 1 - 2 - 3;\n"
        " write(x);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "-4";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesExpressionParentheses) {
    const std::string src =
        "program test;\n"
        "var x : integer;\n"
        "begin\n"
        " x := (7 + 2) / 3;\n"
        " write(x);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "3";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, ThrowsOnExpressionStatement) {
    const std::string src =
        "program test;\n"
        "const x = 300;\n"
        "begin\n"
        " 1 + x + 2 - 5 + (1 + 2);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), ParserException);
}

TEST(CodeGenTests, ThrowsOnAssignmentToExpression) {
    const std::string src =
        "program test;\n"
        "const x = 300;\n"
        "begin\n"
        " 1 := x;\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput;

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), ParserException);
}

TEST(CodeGenTests, HandlesLogicalExpression) {
    const std::string src =
        "program test;\n"
        "begin\n"
        " if 1 = 1 then write(1);\n"
        " if 1 = 2 then write(2);\n"
        " if 1 <> 2 then write(3);\n"
        " if 1 <> 1 then write(4);\n"
        " if 1 < 2 then write(5);\n"
        " if 1 < 1 then write(6);\n"
        " if 1 <= 1 then write(7);\n"
        " if 1 <= 0 then write(8);\n"
        " if 1 > 0 then write(9);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "13579";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

/* ================== Integer/Real Tests ================== */

TEST(CodeGenTests, HandlesRealType_1) {
    const std::string src =
        "program test;\n"
        "var x, y : real;\n"
        "begin\n"
        " x := 1.123;\n"
        " y := 2.223;\n"
        " write(x - y);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "-1.100";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesRealType_2) {
    const std::string src =
        "program test;\n"
        "var X : array [1 .. 4] of real;\n"
        "begin\n"
        " X[1] := -1.1; X[2] := 2.2; X[3] := 3.3;\n"
        " X[4] := X[1] + X[2] + X[3];\n"
        " write(X[4]);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "4.400";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesImplicitConversion) {
    const std::string src =
        "program test;\n"
        "var x, w, z : real;\n"
        "var y : integer;\n"
        "begin\n"
        " x := 1.5;\n"
        " y := 2;\n"
        " w := y;\n"
        " z := x + y + w;\n"
        " write(z);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "5.500";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesExplicitToIntegerConversion) {
    const std::string src =
        "program test;\n"
        "var x : integer; y : real;\n"
        "begin\n"
        " x := to_integer(2.98);\n"
        " write(x);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "2";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesExplicitToRealConversion) {
    const std::string src =
        "program test;\n"
        "var x : integer; y : real;\n"
        "begin\n"
        " x := 5;\n"
        " y := to_real(x);\n"
        " write(y);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "5.000";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, ThrowsOnRealToIntAssignment) {
    const std::string src =
        "program test;\n"
        "var x : real;\n"
        "var y : integer;\n"
        "begin\n"
        " x := 1.5;\n"
        " y := 2.5;\n"
        " write(y);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "-1.100";

    ASSERT_THROW(TestProgram(src, std::nullopt, expectedExitCode, expectedOutput), CodeGenException);
}

/* ================== Complex Program Tests ================== */

TEST(CodeGenTests, HandlesProgramArrayMax) {
    const std::string src =
        "program arrayMax;\n"
        "\n"
        "var I, MAX : integer;\n"
        "var X : array [0 .. 20] of integer;\n"
        "begin\n"
        "  X[0] := 11;\n"
        "  X[1] := 66;\n"
        "  X[2] := 128;\n"
        "  X[3] := 49;\n"
        "  X[4] := 133;\n"
        "  X[5] := 46;\n"
        "  X[6] := 15;\n"
        "  X[7] := 87;\n"
        "  X[8] := 55;\n"
        "  X[9] := 37;\n"
        "  X[10] := 78;\n"
        "  X[11] := 44;\n"
        "  X[12] := 33;\n"
        "  X[13] := 38;\n"
        "  X[14] := 85;\n"
        "  X[15] := 6;\n"
        "  X[16] := 150;\n"
        "  X[17] := 4;\n"
        "  X[18] := 1;\n"
        "  X[19] := 55;\n"
        "  X[20] := 78;\n"
        "\n"
        "  for I := 0 to 20 do begin\n"
        "    writeln(X[I]);\n"
        "  end;\n"
        "  MAX := X[0];\n"
        "  for I := 1 to 20 do begin\n"
        "    if(MAX < X[I]) then MAX := X[I];\n"
        "  end;\n"
        "  writeln(MAX);\n"
        "end.\n";

    const int expectedExitCode = 0;
    const std::string expectedOutput =
        "11\n66\n128\n49\n133\n46\n15\n87\n55\n37\n78\n44\n33\n38\n85\n6\n150\n4\n1\n55\n78\n150\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramArrayAverage) {
    const std::string src =
        "program arrayAverage;\n"
        "\n"
        "var I, TEMP, NUM, SUM : integer;\n"
        "var X : array [-20 .. 20] of integer;\n"
        "begin\n"
        "  for I := -20 to 20 do begin\n"
        "    X[I] := 0;\n"
        "  end;\n"
        "\n"
        "  readln(NUM);\n"
        "\n"
        "  for I := 0 to NUM - 1 do begin\n"
        "    readln(TEMP);\n"
        "    X[TEMP] := X[TEMP] + 1;\n"
        "  end;\n"
        "\n"
        "  SUM := 0;\n"
        "  for I := 20 downto -20 do begin\n"
        "    SUM := SUM + I * X[I];\n"
        "  end;\n"
        "  writeln(SUM div NUM);\n"
        "end.";
    const std::string input = "5 1 2 3 4 5 2";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "3\n";  // (5 + 1 + 2 + 3 + 4 + 5 + 2) / 7 = 22 / 7 = 3

    TestProgram(src, input, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramConsts) {
    const std::string src =
        "program consts;\n"
        "\n"
        "const A =  10;\n"
        "      B = $10;\n"
        "      C = &10;\n"
        "begin\n"
        "  writeln(A);\n"
        "  writeln(B);\n"
        "  writeln(C);\n"
        "end.";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "10\n16\n8\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramExpressions1) {
    const std::string src =
        "program expressions;\n"
        "\n"
        "const\n"
        "    Multiplyer = 5;\n"
        "var\n"
        "    n: integer;\n"
        "\n"
        "begin\n"
        "    readln(n);\n"
        "    n := (n - 1) * Multiplyer + 10;\n"
        "    writeln(n);\n"
        "end.";
    const std::string input = "5";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "30\n";

    TestProgram(src, input, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramExpressions2) {
    const std::string src =
        "program expressions2;\n"
        "\n"
        "var\n"
        "    x: integer;\n"
        "    y: integer;\n"
        "\n"
        "    a: integer;\n"
        "    b: integer;\n"
        "\n"
        "    c: integer;\n"
        "\n"
        "    d: integer;\n"
        "begin\n"
        "    readln(x);\n"
        "    readln(y);\n"
        "\n"
        "    a := x + y;\n"
        "    b := y - x;\n"
        "\n"
        "    writeln(x);\n"
        "    writeln(y);\n"
        "    writeln(a);\n"
        "    writeln(b);\n"
        "\n"
        "    c := (x + a) * (y - b);\n"
        "\n"
        "    writeln(c);\n"
        "\n"
        "    d := a mod b;\n"
        "\n"
        "    writeln(d);\n"
        "end.";
    const std::string input = "5 10";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "5\n10\n15\n5\n100\n0\n";

    TestProgram(src, input, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramFactorial) {
    const std::string src =
        "program factorial;\n"
        "\n"
        "function facti(n : integer) : integer;\n"
        "begin\n"
        "    facti := 1;\n"
        "    while n > 1 do\n"
        "    begin\n"
        "        facti := facti * n;\n"
        "        n := n - 1;\n"
        "    end\n"
        "end;\n"
        "\n"
        "function factr(n : integer) : integer;\n"
        "begin\n"
        "    if n = 1 then\n"
        "        factr := 1\n"
        "    else\n"
        "        factr := n * factr(n-1);\n"
        "end;\n"
        "\n"
        "begin\n"
        "    writeln(facti(5));\n"
        "    writeln(factr(5));\n"
        "end.";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "120\n120\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramFactorialCycle) {
    const std::string src =
        "program factorialCycle;\n"
        "\n"
        "var\n"
        "    n: integer;\n"
        "    f: integer;\n"
        "begin\n"
        "    f := 1;\n"
        "    readln(n);\n"
        "    while(n >= 2) do begin\n"
        "        f := f * n;\n"
        "        n := n - 1;\n"
        "    end;\n"
        "    writeln(f);\n"
        "end.";
    const std::string input = "5";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "120\n";

    TestProgram(src, input, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramFactorialRec) {
    const std::string src =
        "program factorialRec;\n"
        "\n"
        "function fact(n: integer): integer;\n"
        "begin\n"
        "    if (n = 0) then\n"
        "        fact := 1\n"
        "    else\n"
        "        fact := n * fact(n - 1);\n"
        "end;\n"
        "\n"
        "var\n"
        "    n: integer;\n"
        "\n"
        "begin\n"
        "    readln(n);\n"
        "    writeln(fact(n));\n"
        "end.";
    const std::string input = "5";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "120\n";

    TestProgram(src, input, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramFactorization) {
    const std::string src =
        "program factorization;\n"
        "\n"
        "procedure factorization(n: integer);\n"
        "var max, i: integer;\n"
        "begin\n"
        "    writeln(n);\n"
        "\n"
        "    if n < 4 then\n"
        "    begin\n"
        "        writeln(n);\n"
        "        exit;\n"
        "    end;\n"
        "\n"
        "    while ((n mod 2) = 0) do\n"
        "    begin\n"
        "        writeln(2);\n"
        "        n := n div 2;\n"
        "    end;\n"
        "\n"
        "    while ((n mod 3) = 0) do\n"
        "    begin\n"
        "        writeln(3);\n"
        "        n := n div 3;\n"
        "    end;\n"
        "\n"
        "    max := n;\n"
        "    i := 5;\n"
        "    while i <= max do\n"
        "    begin\n"
        "        while ((n mod i) = 0) do\n"
        "        begin\n"
        "            writeln(i);\n"
        "            n := n div i;\n"
        "        end;\n"
        "        i := i + 2;\n"
        "        while ((n mod i) = 0) do\n"
        "        begin\n"
        "            writeln(i);\n"
        "            n := n div i;\n"
        "        end;\n"
        "        i := i + 4;\n"
        "    end;\n"
        "    if n <> 1 then writeln(n);\n"
        "end;\n"
        "\n"
        "begin\n"
        "    factorization(0);\n"
        "    factorization(1);\n"
        "    factorization(2);\n"
        "    factorization(3);\n"
        "    factorization(4);\n"
        "    factorization(5);\n"
        "    factorization(6);\n"
        "    factorization(7);\n"
        "    factorization(8);\n"
        "    factorization(9);\n"
        "    factorization(10);\n"
        "    factorization(11);\n"
        "    factorization(12);\n"
        "    factorization(13);\n"
        "    factorization(14);\n"
        "    factorization(15);\n"
        "    factorization(16);\n"
        "    factorization(17);\n"
        "    factorization(100);\n"
        "    factorization(131);\n"
        "    factorization(133);\n"
        "end.";

    const int expectedExitCode = 0;
    const std::string expectedOutput =
        "0\n"
        "0\n"
        "1\n"
        "1\n"
        "2\n"
        "2\n"
        "3\n"
        "3\n"
        "4\n"
        "2\n"
        "2\n"
        "5\n"
        "5\n"
        "6\n"
        "2\n"
        "3\n"
        "7\n"
        "7\n"
        "8\n"
        "2\n"
        "2\n"
        "2\n"
        "9\n"
        "3\n"
        "3\n"
        "10\n"
        "2\n"
        "5\n"
        "11\n"
        "11\n"
        "12\n"
        "2\n"
        "2\n"
        "3\n"
        "13\n"
        "13\n"
        "14\n"
        "2\n"
        "7\n"
        "15\n"
        "3\n"
        "5\n"
        "16\n"
        "2\n"
        "2\n"
        "2\n"
        "2\n"
        "17\n"
        "17\n"
        "100\n"
        "2\n"
        "2\n"
        "5\n"
        "5\n"
        "131\n"
        "131\n"
        "133\n"
        "7\n"
        "19\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramFibonacci) {
    const std::string src =
        "program fibonacci;\n"
        "\n"
        "function fibonacci(n : integer) : integer;\n"
        "begin\n"
        "    if n < 2 then\n"
        "        fibonacci := n\n"
        "    else\n"
        "        fibonacci := fibonacci(n-1) + fibonacci(n-2);\n"
        "end;\n"
        "\n"
        "begin\n"
        "    writeln(fibonacci(8));\n"
        "    writeln(fibonacci(9));\n"
        "end.";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "21\n34\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramGcd) {
    const std::string src =
        "program gcd;\n"
        "\n"
        "function gcdi(a: integer; b: integer): integer;\n"
        "var tmp: integer;\n"
        "begin\n"
        "    while b <> 0 do\n"
        "    begin\n"
        "        tmp := b;\n"
        "        b := a mod b;\n"
        "        a := tmp;\n"
        "    end;\n"
        "    gcdi := a;\n"
        "end;\n"
        "\n"
        "function gcdr(a: integer; b: integer): integer;\n"
        "var tmp: integer;\n"
        "begin\n"
        "    tmp := a mod b;\n"
        "    if tmp = 0 then\n"
        "    begin\n"
        "        gcdr := b;\n"
        "        exit;\n"
        "    end;\n"
        "    gcdr := gcdr(b, tmp);\n"
        "end;\n"
        "\n"
        "function gcdr_guessing_inner(a: integer; b: integer; c: integer): integer;\n"
        "begin\n"
        "    if ((a mod c) = 0) and ((b mod c) = 0) then\n"
        "    begin\n"
        "        gcdr_guessing_inner := c;\n"
        "        exit;\n"
        "    end;\n"
        "    gcdr_guessing_inner := gcdr_guessing_inner(a, b, c - 1);\n"
        "end;\n"
        "\n"
        "function gcdr_guessing(a: integer; b: integer): integer;\n"
        "begin\n"
        "    gcdr_guessing := gcdr_guessing_inner(a, b, b);\n"
        "end;\n"
        "\n"
        "begin\n"
        "    writeln(gcdi(27*2, 27*3));\n"
        "    writeln(gcdr(27*2, 27*3));\n"
        "    writeln(gcdr_guessing(27*2, 27*3));\n"
        "\n"
        "    writeln(gcdi(5, 7));\n"
        "    writeln(gcdr(5, 7));\n"
        "    writeln(gcdr_guessing(5, 7));\n"
        "\n"
        "    writeln(gcdi(4, 12));\n"
        "    writeln(gcdr(4, 12));\n"
        "    writeln(gcdr_guessing(4, 12));\n"
        "\n"
        "    writeln(gcdi(8, 12));\n"
        "    writeln(gcdr(8, 12));\n"
        "    writeln(gcdr_guessing(8, 12));\n"
        "end.";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "27\n27\n27\n1\n1\n1\n4\n4\n4\n4\n4\n4\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramIndirectRecursion) {
    const std::string src =
        "program indirectrecursion;\n"
        "\n"
        "function isodd(n: integer): integer; forward;\n"
        "function iseven(n: integer): integer;\n"
        "begin\n"
        "    if n > 0 then\n"
        "    begin\n"
        "        iseven := isodd(n - 1);\n"
        "        exit;\n"
        "    end;\n"
        "    iseven := 1;\n"
        "end;\n"
        "\n"
        "function isodd(n: integer): integer;\n"
        "begin\n"
        "    if n > 0 then\n"
        "    begin\n"
        "        isodd := iseven(n - 1);\n"
        "        exit;\n"
        "    end;\n"
        "    isodd := 0;\n"
        "end;\n"
        "\n"
        "begin\n"
        "    writeln(iseven(11));\n"
        "    writeln(isodd(11));\n"
        "end.";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "0\n1\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramInputOutput) {
    const std::string src =
        "program inputOutput;\n"
        "\n"
        "var\n"
        "    n: integer;\n"
        "\n"
        "begin\n"
        "    readln(n);\n"
        "    writeln(n);\n"
        "end.";
    const std::string input = "5";

    const int expectedExitCode = 0;
    const std::string expectedOutput = "5\n";

    TestProgram(src, input, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramIsPrime) {
    const std::string src =
        "program isprime;\n"
        "\n"
        "function isprime(n: integer): integer;\n"
        "var i: integer;\n"
        "begin\n"
        "    if n < 2 then\n"
        "    begin\n"
        "        isprime := 0;\n"
        "        exit;\n"
        "    end;\n"
        "    if n < 4 then\n"
        "    begin\n"
        "        isprime := 1;\n"
        "        exit\n"
        "    end;\n"
        "    if ((n mod 2) = 0) or ((n mod 3) = 0) then\n"
        "    begin\n"
        "        isprime := 0;\n"
        "        exit\n"
        "    end;\n"
        "\n"
        "    isprime := 1;\n"
        "    i := 5;\n"
        "    while i < n do\n"
        "    begin\n"
        "        if ((n mod i) = 0) then\n"
        "        begin\n"
        "            isprime := 0;\n"
        "            exit;\n"
        "        end;\n"
        "        i := i + 2;\n"
        "    end;\n"
        "end;\n"
        "\n"
        "begin\n"
        "    writeln(isprime(0));\n"
        "    writeln(isprime(1));\n"
        "    writeln(isprime(2));\n"
        "    writeln(isprime(3));\n"
        "    writeln(isprime(4));\n"
        "    writeln(isprime(5));\n"
        "    writeln(isprime(6));\n"
        "    writeln(isprime(7));\n"
        "    writeln(isprime(8));\n"
        "    writeln(isprime(9));\n"
        "    writeln(isprime(10));\n"
        "    writeln(isprime(11));\n"
        "    writeln(isprime(12));\n"
        "    writeln(isprime(13));\n"
        "    writeln(isprime(14));\n"
        "    writeln(isprime(15));\n"
        "    writeln(isprime(16));\n"
        "    writeln(isprime(17));\n"
        "    writeln(isprime(17*7));\n"
        "    writeln(isprime(17*11));\n"
        "    writeln(isprime(101));\n"
        "    writeln(isprime(103));\n"
        "end.";

    const int expectedExitCode = 0;
    const std::string expectedOutput =
        "0\n"
        "0\n"
        "1\n"
        "1\n"
        "0\n"
        "1\n"
        "0\n"
        "1\n"
        "0\n"
        "0\n"
        "0\n"
        "1\n"
        "0\n"
        "1\n"
        "0\n"
        "0\n"
        "0\n"
        "1\n"
        "0\n"
        "0\n"
        "1\n"
        "1\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}

TEST(CodeGenTests, HandlesProgramSortBubble) {
    const std::string src =
        "program sortBubble;\n"
        "\n"
        "var I, J, TEMP : integer;\n"
        "var X : array [0 .. 20] of integer;\n"
        "begin\n"
        "  for I := 0 to 20 do begin\n"
        "    X[I] := 20 - I;\n"
        "  end;\n"
        "  for I := 0 to 20 do begin\n"
        "    writeln(X[I]);\n"
        "  end;\n"
        "  for I := 1 to 20 do begin\n"
        "    for J := 20 downto I do begin\n"
        "      if (X[J] < X[J - 1]) then begin\n"
        "\tTEMP := X[J - 1];\n"
        "        X[J - 1] := X[J];\n"
        "\tX[J] := TEMP;\n"
        "      end\n"
        "    end\n"
        "  end;\n"
        "  for I := 0 to 20 do begin\n"
        "    writeln(X[I]);\n"
        "  end\n"
        "end.";

    const int expectedExitCode = 0;
    const std::string expectedOutput =
        "20\n"
        "19\n"
        "18\n"
        "17\n"
        "16\n"
        "15\n"
        "14\n"
        "13\n"
        "12\n"
        "11\n"
        "10\n"
        "9\n"
        "8\n"
        "7\n"
        "6\n"
        "5\n"
        "4\n"
        "3\n"
        "2\n"
        "1\n"
        "0\n"
        "0\n"
        "1\n"
        "2\n"
        "3\n"
        "4\n"
        "5\n"
        "6\n"
        "7\n"
        "8\n"
        "9\n"
        "10\n"
        "11\n"
        "12\n"
        "13\n"
        "14\n"
        "15\n"
        "16\n"
        "17\n"
        "18\n"
        "19\n"
        "20\n";

    TestProgram(src, std::nullopt, expectedExitCode, expectedOutput);
}
