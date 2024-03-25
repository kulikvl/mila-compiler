#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "utils/Utils.hpp"

/**
 * @note No fancy view nor OOP, just a simple cli args parser to make compiler a bit more user-friendly
 */
class SimpleConsoleView {
    /**
     * @brief parseArgs sets this flag if all arguments are valid and ready to run
     */
    bool readyToRun = false;

    /**
     * @brief Whether to dump grammar rules used while parsing and final ast tree
     */
    bool verbose = false;

    /**
     * @brief .mila source file
     */
    std::string inputFileName;

    /**
     * @brief output executable file
     */
    std::string outputFileName;

    void showHelp() const;

   public:
    /**
     * @brief Starts the application
     */
    int run(const std::vector<std::string>& args);

    /**
     * @brief Parses the command line arguments
     */
    int parseArgs(const std::vector<std::string>& args);
};