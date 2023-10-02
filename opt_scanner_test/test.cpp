#define CRT_SECURE_NO_WARNINGS

#include "pch.h"

//#include "Candle.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Isolate desired tests with this filter
    ::testing::GTEST_FLAG(filter) = "DBTests*";

    // Exclude tests here
    //::testing::GTEST_FLAG(filter) = "-BenchmarkTests*";

    return RUN_ALL_TESTS();
}