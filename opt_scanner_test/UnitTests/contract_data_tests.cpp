#define _CRT_SECURE_NO_WARNINGS

#include "ContractData.h"
#include "../pch.h"

#include "../MockClasses/MockClient.h"
#include "../MockClasses/MockWrapper.h"


using namespace testing;

// Test to ensure all candle vectors are filled with accurate amounts when fed candle data
TEST(ContractDataTests, updateDataTest) {
	// Use the wrapper to request randomly generated candles
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	Contract con;
	con.symbol = "SPX";
	con.secType = "IND";

	// Will produce 720 candles
	std::string endDateTime = "20230705 09:30:00";
	std::string durationStr = "3600 S";
	std::string barSizeSetting = "5 secs";

	mClient.reqHistoricalData(1234, con, endDateTime, durationStr, barSizeSetting, "", 0, 0, false);

	while (mWrapper.notDone()) continue;

	auto data = mWrapper.getHistoricCandles();

	// Values for test comparison
	double maxPrice = data[0]->high();
	double minPrice = data[0]->low();
	double lastPrice = data.back()->close();
	long long totalVol = data[0]->volume();

	TickerId reqId = data[0]->reqId();
	ContractData cd(reqId, std::move(data[0]));

	EXPECT_EQ(cd.contractId(), 1234);

	for (size_t i = 1; i < data.size(); i++) {
		maxPrice = std::max(maxPrice, data[i]->high());
		minPrice = std::min(minPrice, data[i]->low());
		totalVol += data[i]->volume();
		cd.updateData(std::move(data[i]));
	}

	EXPECT_EQ(cd.fiveSecData().size(), 720);
	EXPECT_EQ(cd.thirtySecData().size(), 120);
	EXPECT_EQ(cd.oneMinData().size(), 60);
	EXPECT_EQ(cd.fiveMinData().size(), 12);

	EXPECT_EQ(cd.dailyLow(), minPrice);
	EXPECT_EQ(cd.dailyHigh(), maxPrice);
	EXPECT_EQ(cd.currentPrice(), lastPrice);
	EXPECT_EQ(cd.totalVol(), totalVol);

	// Test return functionality for last 30 minutes of candles
	EXPECT_EQ(cd.candlesLast30Minutes().size(), 360);
}

TEST(ContractDataTests, AlertSystemTest) {
	// Will manually add candles to generate isolated alerts
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	std::vector<std::unique_ptr<Candle>> candles;

	for (size_t i = 0; i < 720; i++) {
		if (i == 300) {
			candles.push_back(std::move(std::make_unique<Candle>(11, i, 3, 4, 1, 3, 1000000)));
		}
		else {
			candles.push_back(std::move(std::make_unique<Candle>(11, i, 3, 4, 1, 3, 1000)));
		}
	}

	TickerId reqId = candles[0]->reqId();
	ContractData cd(reqId, std::move(candles[0]));

	TimeFrame testTF;
	double testStDev = 0;
	double volume = 0;
	int alertCount = 0;
	std::tuple<TimeFrame, double, double> test1;
	std::tuple<TimeFrame, double, double> test2;
	std::tuple<TimeFrame, double, double> test3;
	std::tuple<TimeFrame, double, double> test4;
	std::vector<std::tuple<TimeFrame, double, double>> vTests;

	// Register the alert
	cd.registerAlert([&]
	(TimeFrame tf, std::shared_ptr<Candle> candle) {
			volume = static_cast<double>(candle->volume());
			testStDev = cd.volStDev(tf).numStDev(volume);
			testTF = tf;
			vTests.push_back(std::make_tuple(tf, volume, testStDev));
			alertCount++;
		});

	for (size_t i = 1; i < candles.size(); i++) {
		cd.updateData(std::move(candles[i]));
	}

	EXPECT_TRUE(std::get<0>(vTests[0]) == TimeFrame::FiveSecs && std::get<1>(vTests[0]) == 1000000 && std::get<2>(vTests[0]) > 2);
	EXPECT_TRUE(std::get<0>(vTests[1]) == TimeFrame::ThirtySecs && std::get<1>(vTests[1]) >= 1000000 && std::get<2>(vTests[1]) > 2);
	EXPECT_TRUE(std::get<0>(vTests[2]) == TimeFrame::OneMin && std::get<1>(vTests[2]) >= 1000000 && std::get<2>(vTests[2]) > 2);
	EXPECT_TRUE(std::get<0>(vTests[3]) == TimeFrame::FiveMin && std::get<1>(vTests[3]) >= 1000000 && std::get<2>(vTests[3]) > 2);
	EXPECT_EQ(alertCount, 4); // Four for each timeframe
}