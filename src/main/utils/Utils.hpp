#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "ast/CodeGenerator.hpp"

/**
 * @brief Various constructs to simplify life and increase reuse
 */
namespace Utils {

/**
 * @brief Program execution result
 */
struct ProgramRunResult {
    int exitCode;
    std::string output;

    ProgramRunResult(int exitCode, std::string output);
};

/**
 * @brief Execute shell command
 * @param shellCmd Command to execute
 * @returns Program execution result
 */
ProgramRunResult exec(const std::string& shellCmd);

/**
 * @brief Convert argc and argv to vector of strings
 * @param argc Argument count
 * @param argv Argument values
 * @returns Vector of strings
 */
std::vector<std::string> convertArgvToVector(int argc, char* argv[]);

/**
 * @brief Check if a file has a given extension
 * @param filename File name
 * @param extension Extension to check
 * @returns True if the file has the given extension, false otherwise
 */
bool hasExtension(const std::string& filename, const std::string& extension);

namespace LLVM {

/**
 * @brief Generate index out of bounds check
 * @param arrayName Array name for error message
 * @param indexV Index value
 * @param lowerBoundV Lower bound value
 * @param upperBoundV Upper bound value
 * @param gen Code generation context
 */
void generateIndexOutOfBoundsCheck(const std::string& arrayName, llvm::Value* indexV, llvm::Value* lowerBoundV, llvm::Value* upperBoundV,
                                   GenContext& gen);

}  // namespace LLVM

}  // namespace Utils
