#define _CRT_SECURE_NO_WARNINGS

#include "SQLSchema.h"
#include "CandleRoutes.h"
#include "../pch.h"

#include "../MockClasses/MockClient.h"
#include "../MockClasses/MockWrapper.h"

using namespace testing;

TEST(DBTests, connectToDB) {
	nanodbc::connection conn = OptionDB::connectToDB();
	OptionDB::CandleTables::setTable(conn, TimeFrame::FiveSecs);

	// Create some historic candles with the mock wrapper
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
	std::string durationStr = "100 S";
	std::string barSizeSetting = "5 secs";

	mClient.reqHistoricalData(4050, con, endDateTime, durationStr, barSizeSetting, "", 0, 0, false);

	while (mWrapper.notDone()) continue;

	auto data = mWrapper.getHistoricCandles();
	std::vector<std::shared_ptr<Candle>> testData;

	for (auto& c : data) {
		std::shared_ptr<Candle> candle = std::make_shared<Candle>(std::move(*c));
		testData.push_back(candle);
	}

	EXPECT_EQ(testData.size(), 20);

	for (auto& c : testData) {
		OptionDB::CandleTables::post(conn, c, TimeFrame::FiveSecs);
	}

	std::vector<Candle> sqlData = OptionDB::CandleTables::get(conn, TimeFrame::FiveSecs);

	for (size_t i = 0; i < sqlData.size(); i++) {
		ASSERT_TRUE(sqlData[i].reqId() == testData[i]->reqId());
		ASSERT_TRUE(sqlData[i].time() == testData[i]->time());
		ASSERT_TRUE(sqlData[i].volume() == testData[i]->volume());

		// Since the sql values of the prices are rounded, they will come out to a slightly different value
		// need to test for accuracy rather than equality
		ASSERT_TRUE(abs(sqlData[i].open() - testData[i]->open()) < 0.01);
		ASSERT_TRUE(abs(sqlData[i].high() - testData[i]->high()) < 0.01);
		ASSERT_TRUE(abs(sqlData[i].low() - testData[i]->low()) < 0.01);
		ASSERT_TRUE(abs(sqlData[i].close() - testData[i]->close()) < 0.01);
	}
}