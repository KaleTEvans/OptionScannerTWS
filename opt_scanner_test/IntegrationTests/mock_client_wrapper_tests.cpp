#define _CRT_SECURE_NO_WARNINGS

#include "../pch.h"

#include "../MockTwsApi/MockWrapper.h"
#include "../MockTwsApi/MockClient.h"

using namespace testing;

// Ensure the mock client returns the correct time
TEST(ClientWrapperTest, currentTime) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	auto currentTime = std::chrono::system_clock::now().time_since_epoch();
	std::time_t unixTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
	long time = static_cast<long>(unixTime);

	mClient.reqCurrentTime();
	EXPECT_EQ(mWrapper.getCurrentTime(), time);
}

// Check right amount of candlesticks are recieved based on request
TEST(ClientWrapperTest, historicalCandleSize) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	Contract con;
	con.symbol = "SPX";
	con.secType = "IND";

	mClient.reqHistoricalData(100, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);
	EXPECT_EQ(mWrapper.getHistoricCandles().size(), 720);

	mClient.reqHistoricalData(100, con, "20230705 08:30:00", "21600 S", "30 secs", "", 0, 0, false);
	EXPECT_EQ(mWrapper.getHistoricCandles().size(), 720);

	mClient.reqHistoricalData(100, con, "20230705 08:30:00", "43200 S", "60 secs", "", 0, 0, false);
	EXPECT_EQ(mWrapper.getHistoricCandles().size(), 720);
}

// Test to ensure that the mock historical data is an accurate representation of candlesticks for underlying price
TEST(ClientWrapperTest, historicalDataAccuracy) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	double spxRefPrice = 4580;
	long spxRefVol = 50000000;

	double priceRange = 20;
	long volumeRange = 1000000;

	Contract con;
	con.symbol = "SPX";
	con.secType = "IND";

	std::string endDateTime = "20230705 09:30:00";
	std::string durationStr = "3600 S";
	std::string barSizeSetting = "5 secs";

	mClient.reqHistoricalData(4050, con, endDateTime, durationStr, barSizeSetting, "", 0, 0, false);

	while (mWrapper.notDone()) continue;

	const auto data = mWrapper.getHistoricCandles();

	for (const auto& candlePtr : data) {
		Candle& candle = *candlePtr;

		EXPECT_TRUE((candle.getOpen() >= (spxRefPrice - priceRange) && candle.getOpen() <= (spxRefPrice + priceRange)));
		EXPECT_TRUE((candle.getHigh() >= (spxRefPrice - priceRange) && candle.getHigh() <= (spxRefPrice + priceRange)));
		EXPECT_TRUE((candle.getLow() >= (spxRefPrice - priceRange) && candle.getLow() <= (spxRefPrice + priceRange)));
		EXPECT_TRUE((candle.getClose() >= (spxRefPrice - priceRange) && candle.getClose() <= (spxRefPrice + priceRange)));
		EXPECT_TRUE((candle.getVolume() >= (spxRefVol - volumeRange) && candle.getVolume() <= (spxRefVol + volumeRange)));
	}
}

// Variables to use for option representation
class HistOptionDataAccuracyTest : public ::testing::TestWithParam<std::tuple<int, int, double>> {};

INSTANTIATE_TEST_SUITE_P(mockOptionData, HistOptionDataAccuracyTest,
	::testing::Values(
		std::make_tuple(4581, 4590, 300.0), // 2 strikes OTM
		std::make_tuple(4579, 4591, 800.0), // 3 strikes ITM
		std::make_tuple(4570, 4570, 500.0), // ATM
		std::make_tuple(4576, 4595, 100.0), // 4 strikes OTM
		std::make_tuple(4571, 4600, 0.0)   // 5 strikes OTM
	)
);

TEST_P(HistOptionDataAccuracyTest, TestHistOptionDataAccuracy) {
	const auto& params = GetParam();
	int underlying = std::get<0>(params);
	int optPrice = std::get<1>(params);
	double refVol = std::get<2>(params);

	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	Contract con;
	con.symbol = "SPX";
	con.secType = "OPT";

	mWrapper.setMockUnderlying(underlying);

	double refPrice = refVol / 100;
	double priceRange = 0.5;
	long volumeRange = 250;

	mClient.reqHistoricalData(optPrice, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);

	while (mWrapper.notDone()) continue;
	const auto data = mWrapper.getHistoricCandles();

	for (const auto& candlePtr : data) {
		Candle& candle = *candlePtr;

		ASSERT_TRUE(candle.getOpen() >= (refPrice - priceRange) && candle.getOpen() <= (refPrice + priceRange));
		ASSERT_TRUE(candle.getHigh() >= (refPrice - priceRange) && candle.getHigh() <= (refPrice + priceRange));
		ASSERT_TRUE(candle.getLow() >= (refPrice - priceRange) && candle.getLow() <= (refPrice + priceRange));
		ASSERT_TRUE(candle.getClose() >= (refPrice - priceRange) && candle.getClose() <= (refPrice + priceRange));
		ASSERT_TRUE(candle.getVolume() >= (refVol - volumeRange) && candle.getVolume() <= (refVol + volumeRange));
	}
}