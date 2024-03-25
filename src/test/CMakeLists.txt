# Download and build Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(my_tests LexerTests.cpp
        ParserTests.cpp
        CodeGenTests.cpp
        UtilsTests.cpp)
target_link_libraries(my_tests PRIVATE mila_lib gtest_main pthread)
file(COPY ${CMAKE_SOURCE_DIR}/external/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()

include(GoogleTest)
gtest_discover_tests(my_tests)
