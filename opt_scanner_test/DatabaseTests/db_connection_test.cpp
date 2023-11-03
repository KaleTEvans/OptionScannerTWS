#define _CRT_SECURE_NO_WARNINGS

#include "SQLSchema.h"
#include "CandleRoutes.h"
#include "../pch.h"

#include "../MockClasses/MockClient.h"
#include "../MockClasses/MockWrapper.h"

using namespace testing;

TEST(DBTests, connectToDB) {
	nanodbc::connection conn = OptionDB::connectToDB();
	OptionDB::CandleTables::setTable(conn);

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
	//std::vector<std::shared_ptr<Candle>> testData;
	std::vector<OptionDB::CandleTables::CandleForDB> dbCandles;

	for (auto& c : data) {
		OptionDB::CandleTables::CandleForDB cdb(c->reqId(), c->date(), c->time(), c->open(), c->high(), c->low(), c->close(), c->volume());

		dbCandles.push_back(cdb);
	}

	EXPECT_EQ(dbCandles.size(), 20);

	for (auto& c : dbCandles) {
		OptionDB::CandleTables::post(conn, c, TimeFrame::FiveSecs);
	}

	std::vector<Candle> sqlData = OptionDB::CandleTables::get(conn, TimeFrame::FiveSecs);

	for (size_t i = 0; i < sqlData.size(); i++) {
		ASSERT_TRUE(sqlData[i].reqId() == dbCandles[i].reqId_);
		ASSERT_TRUE(sqlData[i].time() == dbCandles[i].time_);
		ASSERT_TRUE(sqlData[i].volume() == dbCandles[i].volume_);

		// Since the sql values of the prices are rounded, they will come out to a slightly different value
		// need to test for accuracy rather than equality
		ASSERT_TRUE(abs(sqlData[i].open() - dbCandles[i].open_) < 0.01);
		ASSERT_TRUE(abs(sqlData[i].high() - dbCandles[i].high_) < 0.01);
		ASSERT_TRUE(abs(sqlData[i].low() - dbCandles[i].low_) < 0.01);
		ASSERT_TRUE(abs(sqlData[i].close() - dbCandles[i].close_) < 0.01);
	}
}