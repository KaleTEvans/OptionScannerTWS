#define _CRT_SECURE_NO_WARNINGS
//#define CDTEST

#include "../pch.h"

#include "../MockTwsApi/MockClient.h"
#include "../MockTwsApi/MockWrapper.h"
#include "ContractData.h"

#include <memory>

using namespace testing;

// Create a separate implementation of the updateData function
class MockContractData : public ContractData {
public:

	//std::mutex cdMutex;

	MockContractData(TickerId reqId, std::unique_ptr<Candle> initData) :
		ContractData(reqId, std::move(initData)) {}

	void candleFunctions(TimeFrame tf, std::shared_ptr<Candle> c = nullptr) {
		//std::lock_guard<std::mutex> lock(cdMutex);
		std::shared_ptr<Candle> addOn = c;
		vector<std::shared_ptr<Candle>>* candleVec = nullptr;
		StandardDeviation* sdPrice = nullptr;
		StandardDeviation* sdVol = nullptr;

		if (tf == TimeFrame::FiveSecs) {
			candleVec = &fiveSecCandles;
			sdPrice = &sd5Sec;
			sdVol = &sdVol5Sec;
		}
		else if (tf == TimeFrame::ThirtySecs) {
			addOn = createNewBars(contractId_, 6, fiveSecCandles);
			candleVec = &thirtySecCandles;
			sdPrice = &sd30Sec;
			sdVol = &sdVol30Sec;
		}
		else if (tf == TimeFrame::OneMin) {
			addOn = createNewBars(contractId_, 2, thirtySecCandles);
			candleVec = &oneMinCandles;
			sdPrice = &sd1Min;
			sdVol = &sdVol1Min;
		}
		else {
			addOn = createNewBars(contractId_, 5, oneMinCandles);
			candleVec = &fiveMinCandles;
			sdPrice = &sd5Min;
			sdVol = &sdVol5Min;
		}

		candleVec->push_back(addOn);
		sdPrice->addValue(addOn->high() - addOn->low());
		sdVol->addValue(addOn->volume());

		delete candleVec, delete sdPrice, delete sdVol;
	}

	void mockUpdateData(std::unique_ptr<Candle> c) {
		// Switch to shared pointer
		std::shared_ptr<Candle> fiveSec{ std::move(c) };

		candleFunctions(TimeFrame::FiveSecs, fiveSec);

		// Update daily high and low values to check relative price
		dailyHigh_ = max(dailyHigh_, fiveSec->high());
		dailyLow_ = min(dailyLow_, fiveSec->low());

		updateComparisons();

		// Using the length of the 5 sec array, we will determine if any new candles should be added to the other arrays
		// 6 increments for the 30 sec
		if (fiveSecCandles.size() % 6 == 0 && fiveSecCandles.size() > 0) candleFunctions(TimeFrame::ThirtySecs);

		// Now we'll reference the 30 sec array for the 1min, so we only need to use increments of 2
		if (thirtySecCandles.size() > 0 && thirtySecCandles.size() % 2 == 0) {
			candleFunctions(TimeFrame::OneMin);

			///////////////// Update cumulative volume for historical records///////////////
			if (cumulativeVolume_.empty()) {
				std::pair<long, long> p = { oneMinCandles.back()->time(), oneMinCandles.back()->volume() };
				cumulativeVolume_.push_back(p);
			}
			else {
				int totalVol = oneMinCandles.back()->volume() + cumulativeVolume_.back().second;
				std::pair<long, long> p{ oneMinCandles.back()->time(), totalVol };
				cumulativeVolume_.push_back(p);
			}
		}

		// Referencing the 1 min for the 5min array we can use increments of 5
		if (oneMinCandles.size() > 0 && oneMinCandles.size() % 5 == 0) {
			candleFunctions(TimeFrame::FiveMin);

			// Every 30 minutes, update the local high and low. Temp high and low will serve to track these values in between
			tempHigh_ = max(tempHigh_, fiveMinCandles.back()->high());
			tempLow_ = min(tempLow_, fiveMinCandles.back()->low());

			if (fiveMinCandles.size() % 6 == 0) {
				localHigh_ = tempHigh_;
				localLow_ = tempLow_;
				tempHigh_ = 0;
				tempLow_ = 10000;
			}
		}
	}
};

TEST(BenchmarkTests, MockContractDataTest) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	Contract con;
	con.symbol = "SPX";
	con.secType = "IND";

	// Will produce 720 candles
	std::string endDateTime = "20230705 09:30:00";
	std::string durationStr = "3600 S";
	std::string barSizeSetting = "5 secs";

	////////////////////// Warmups ////////////////////////

	mClient.reqHistoricalData(1111, con, endDateTime, durationStr, barSizeSetting, "", 0, 0, false);
	while (mWrapper.notDone()) continue;
	auto data = mWrapper.getHistoricCandles();
	mWrapper.setmDone(false);

	std::cout << data[0]->reqId() << std::endl;

	TickerId reqId = data[0]->reqId();
	MockContractData mcd(reqId, std::move(data[0]));

	for (size_t i = 1; i < data.size(); i++) {
		mcd.mockUpdateData(std::move(data[i]));
	}

	/*mClient.reqHistoricalData(2222, con, endDateTime, durationStr, barSizeSetting, "", 0, 0, false);
	while (mWrapper.notDone()) continue;
	auto data2 = mWrapper.getHistoricCandles();

	std::cout << std::endl;
	std::cout << data2.size() << std::endl;
	std::cout << data2[0]->reqId() << std::endl;
	std::cout << data2[1]->reqId() << std::endl;

	TickerId reqId2 = data2[0]->reqId();
	ContractData cd(reqId2, std::move(data2[0]));

	for (size_t i = 1; i < data2.size(); i++) {
		cd.updateData(std::move(data2[i]));
	}

	EXPECT_EQ(mcd.contractId(), 1111);
	EXPECT_EQ(cd.contractId(), 2222);*/
	/*EXPECT_EQ(mcd.fiveSecData().size(), 720);
	EXPECT_EQ(cd.fiveSecData().size(), 720);*/
}

//mClient.reqHistoricalData(2222, con, endDateTime, durationStr, barSizeSetting, "", 0, 0, false);
//while (mWrapper.notDone()) continue;
//auto data2 = mWrapper.getHistoricCandles();
//
//TickerId reqId2 = data2[0]->reqId();
//ContractData cd(reqId2, std::move(data2[0]));
//
//for (size_t i = 1; i < data2.size(); i++) {
//	cd.updateData(std::move(data2[i]));
//}
//
////////////////////////// Benchmark Testing ///////////////////////////////
//
///////////////////////////// Mock Test With new update function /////////////////
//
//mClient.reqHistoricalData(3333, con, endDateTime, durationStr, barSizeSetting, "", 0, 0, false);
//while (mWrapper.notDone()) continue;
//auto dataTestMock = mWrapper.getHistoricCandles();
//mWrapper.setmDone(false);
//
//m_beg = Clock::now();
//
//TickerId reqIdTestMock = dataTestMock[0]->reqId();
//MockContractData mcdTestMock(reqIdTestMock, std::move(dataTestMock[0]));
//
//for (size_t i = 1; i < dataTestMock.size(); i++) {
//	mcdTestMock.updateData(std::move(dataTestMock[i]));
//}
//
//auto elapsed1 = std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();