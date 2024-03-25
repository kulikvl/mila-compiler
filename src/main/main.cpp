#include "ast/CodeGenerator.hpp"
#include "ui/SimpleConsoleView.hpp"
#include "utils/Utils.hpp"

int main(int argc, char* argv[]) {
    std::vector<std::string> args = Utils::convertArgvToVector(argc, argv);
    SimpleConsoleView view;
    return view.run(args);
}
