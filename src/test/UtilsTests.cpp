#include <gtest/gtest.h>
#include "utils/Utils.hpp"

TEST(CodeGenTests, HandlesExecCommmandSuccess) {
    Utils::ProgramRunResult programResult = Utils::exec("ls -l");

    ASSERT_TRUE(programResult.exitCode == 0);
    ASSERT_FALSE(programResult.output.empty());
}

TEST(CodeGenTests, HandlesExecCommmandError) {
    Utils::ProgramRunResult programResult = Utils::exec("ls -[ 2>/dev/null");

    EXPECT_EQ(2, programResult.exitCode);
}
