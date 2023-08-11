#define _CRT_SECURE_NO_WARNINGS

#include "../pch.h"
//#include "gtest/gtest.h"
#include "Candle.h"

TEST(CandleTest, ConvertDateToUnix) {
    Candle candle(789, "20230806 12:45:30", 120.0, 130.0, 115.0, 125.0, 2000, 2, 125.0, 0);

    // Assuming "20230806 12:45:30" corresponds to 1699320330 in Unix timestamp
    EXPECT_EQ(candle.time(), 1691347530);
}

TEST(CandleTest, ConvertUnixToDate) {
    Candle candle(789, 1691347530, 120.0, 130.0, 115.0, 125.0, 2000, 125.0, 0);

    std::string date = "20230806 13:45:30"; // Updated expected date as a std::string

    // Convert the IBString to a std::string for comparison
    std::string candleDate = candle.date();

    EXPECT_EQ(candleDate, date);
}