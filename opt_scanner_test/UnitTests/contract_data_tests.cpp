#define _CRT_SECURE_NO_WARNINGS

#include "../pch.h"

#include "../MockTwsApi/MockClient.h"
#include "../MockTwsApi/MockWrapper.h"
#include "ContractData.h"

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

	TickerId reqId = data[0]->reqId();
	ContractData cd(reqId, std::move(data[0]));

	EXPECT_EQ(cd.contractId(), 1234);

	for (size_t i = 1; i < data.size(); i++) {
		cd.updateData(std::move(data[i]));
	}

	 EXPECT_EQ(cd.fiveSecData().size(), 720);
	 EXPECT_EQ(cd.thirtySecData().size(), 120);
	 EXPECT_EQ(cd.oneMinData().size(), 60);
	 EXPECT_EQ(cd.fiveMinData().size(), 12);
}