#define _CRT_SECURE_NO_WARNINGS

#include "../pch.h"

#include "../MockClasses/MockWrapper.h"
#include "../MockClasses/MockClient.h"

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

		EXPECT_TRUE((candle.open() >= (spxRefPrice - priceRange) && candle.open() <= (spxRefPrice + priceRange)));
		EXPECT_TRUE((candle.high() >= (spxRefPrice - priceRange) && candle.high() <= (spxRefPrice + priceRange)));
		EXPECT_TRUE((candle.low() >= (spxRefPrice - priceRange) && candle.low() <= (spxRefPrice + priceRange)));
		EXPECT_TRUE((candle.close() >= (spxRefPrice - priceRange) && candle.close() <= (spxRefPrice + priceRange)));
		EXPECT_TRUE((candle.volume() >= (spxRefVol - volumeRange) && candle.volume() <= (spxRefVol + volumeRange)));
	}
}

// Variables to use for option representation
class ClientWrapperTest : public TestWithParam<std::tuple<int, int, double>> {};

INSTANTIATE_TEST_CASE_P(SampleOptionData, ClientWrapperTest,
	Values(
		std::make_tuple(4581, 4590, 300.0), // 2 strikes OTM
		std::make_tuple(4579, 4591, 800.0), // 3 strikes ITM
		std::make_tuple(4570, 4570, 500.0), // ATM
		std::make_tuple(4576, 4595, 100.0), // 4 strikes OTM
		std::make_tuple(4571, 4600, 0.0)   // 5 strikes OTM
	)
);

TEST_P(ClientWrapperTest, TestHistOptionDataAccuracy) {
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

		ASSERT_TRUE(candle.open() >= (refPrice - priceRange) && candle.open() <= (refPrice + priceRange));
		ASSERT_TRUE(candle.high() >= (refPrice - priceRange) && candle.high() <= (refPrice + priceRange));
		ASSERT_TRUE(candle.low() >= (refPrice - priceRange) && candle.low() <= (refPrice + priceRange));
		ASSERT_TRUE(candle.close() >= (refPrice - priceRange) && candle.close() <= (refPrice + priceRange));
		ASSERT_TRUE(candle.volume() >= (refVol - volumeRange) && candle.volume() <= (refVol + volumeRange));
	}
}


// Test to ensure that the realTimeBars are being output in 5 second intervals for each request
TEST(ClientWrapperTest, realTimeBarsOutput) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	Contract con;
	con.symbol = "SPX";
	con.secType = "OPT";

	//mWrapper.showRealTimeDataOutput();

	mClient.reqRealTimeBars(4580, con, 0, "", true);
	mClient.reqRealTimeBars(4581, con, 0, "", true);
	mClient.reqRealTimeBars(4576, con, 0, "", true);
	mClient.reqRealTimeBars(4590, con, 0, "", true);

	mWrapper.setBufferCapacity(4);
	mClient.setCandleInterval(10);

	std::unordered_map<int, std::vector<std::unique_ptr<Candle>>> contracts;
	int i = 0;

	while (i <= 50) {
		std::unique_lock<std::mutex> lock(mWrapper.getWrapperMutex());
		mWrapper.getWrapperConditional().wait(lock, [&] { return mWrapper.checkMockBufferFull(); });
		std::vector<std::unique_ptr<Candle>> temp = mWrapper.getProcessedFiveSecCandles();
		for (auto& i : temp) {
			EXPECT_TRUE((i->reqId() == 4580 || i->reqId() == 4581 || i->reqId() == 4576 || i->reqId() == 4590));
			contracts[i->reqId()].push_back(std::move(i));
		}
		lock.unlock();

		i++;
	}

	mClient.cancelRealTimeBars();

	for (auto& it : contracts) {
		std::vector<std::unique_ptr<Candle>> temp = std::move(it.second);
		for (size_t i = 1; i < temp.size(); i++) {
			EXPECT_TRUE(temp[i]->time() - temp[i - 1]->time() == 5);
			EXPECT_TRUE(temp[i]->high() >= temp[i]->low());
		}
	}
}

// Try intentionally setting the buffer size to be higher than the active requests
TEST(ClientWrapperTest, realTimeBarsEdgeCase) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	Contract con;
	con.symbol = "SPX";
	con.secType = "OPT";

	//mWrapper.showRealTimeDataOutput();

	mClient.reqRealTimeBars(4580, con, 0, "", true);
	mClient.reqRealTimeBars(4590, con, 0, "", true);

	mWrapper.setBufferCapacity(4); // Intentionally set capacity higher
	mClient.setCandleInterval(10); // 10 ms between candle generation

	std::unordered_map<int, std::vector<std::unique_ptr<Candle>>> contracts;
	int i = 0;

	while (i <= 50) {

		std::unique_lock<std::mutex> lock(mWrapper.getWrapperMutex());
		mWrapper.getWrapperConditional().wait(lock, [&] { return mWrapper.checkMockBufferFull(); });
		std::vector<std::unique_ptr<Candle>> temp = mWrapper.getProcessedFiveSecCandles();
		for (auto& i : temp) {
			EXPECT_TRUE((i->reqId() == 4580 || i->reqId() == 4590));
			contracts[i->reqId()].push_back(std::move(i));
		}
		lock.unlock();

		i++;
	}

	mClient.cancelRealTimeBars();

	EXPECT_EQ(contracts.size(), 2);
	EXPECT_EQ(contracts.size(), mWrapper.getBufferCapacity());
	EXPECT_TRUE(contracts[4580].size() > 10);
}