//#define _CRT_SECURE_NO_WARNINGS
//
#define BOOST_TEST_LOG_LEVEL all
#define BOOST_TEST_MODULE client_wrapper_test
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
//#include <boost/test/data/monomorphic.hpp>
//#include <boost/test/tools/assertion_result.hpp>
//#include <boost/test/tools/floating_point_comparison.hpp>

#include "MockClient.h"

namespace utf = boost::unit_test;
namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(ClientWrapperTests)

BOOST_AUTO_TEST_CASE(currentTime) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	auto currentTime = std::chrono::system_clock::now().time_since_epoch();
	std::time_t unixTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
	long time = static_cast<long>(unixTime);

	mClient.reqCurrentTime();

	BOOST_REQUIRE_EQUAL(mWrapper.getCurrentTime(), time);
}

const auto test_data_hist = data::make({
	std::make_tuple("20230705 09:30:00", "3600 S", "5 secs"),
	std::make_tuple("20230705 08:30:00", "21600 S", "30 secs"),
	std::make_tuple("20230705 08:30:00", "43200 S", "60 secs")
});

BOOST_DATA_TEST_CASE(historicalData, test_data_hist, endDateTime, durationStr, barSizeSetting) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	Contract con;
	con.symbol = "SPX";
	con.secType = "IND";

	mClient.reqHistoricalData(100, con, endDateTime, durationStr, barSizeSetting, "", 0, 0, false);

	BOOST_TEST(mWrapper.getHistoricCandles().size() == 720);
}

BOOST_AUTO_TEST_CASE(historicalDataAccuracy) {
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
		CandleStick& candle = *candlePtr;

		BOOST_TEST((candle.getOpen() >= (spxRefPrice - priceRange) && candle.getOpen() <= (spxRefPrice + priceRange)));
		BOOST_TEST((candle.getHigh() >= (spxRefPrice - priceRange) && candle.getHigh() <= (spxRefPrice + priceRange)));
		BOOST_TEST((candle.getLow() >= (spxRefPrice - priceRange) && candle.getLow() <= (spxRefPrice + priceRange)));
		BOOST_TEST((candle.getClose() >= (spxRefPrice - priceRange) && candle.getClose() <= (spxRefPrice + priceRange)));
		BOOST_TEST((candle.getVolume() >= (spxRefVol - volumeRange) && candle.getVolume() <= (spxRefVol + volumeRange)));
	}
}

const auto test_data_options = data::make({
	// Underlying, Stirke, RefVol
	std::make_tuple(4581, 4590, 300.0), // 2 strikes OTM
	std::make_tuple(4579, 4591, 800.0), // 3 strikes ITM
	std::make_tuple(4570, 4570, 500.0), // ATM
	std::make_tuple(4576, 4595, 100.0), // 4 strikes OTM
	std::make_tuple(4571, 4600, 0.0) // 5 strikes OTM
}); // Test pricing for one call and one put, SPX reference is set to 4581

BOOST_DATA_TEST_CASE(histOptionDataAccuracy, test_data_options, underlying, optPrice, refVol) {
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
		CandleStick& candle = *candlePtr;

		BOOST_TEST((candle.getOpen() >= (refPrice - priceRange) && candle.getOpen() <= (refPrice + priceRange)));
		BOOST_TEST((candle.getHigh() >= (refPrice - priceRange) && candle.getHigh() <= (refPrice + priceRange)));
		BOOST_TEST((candle.getLow() >= (refPrice - priceRange) && candle.getLow() <= (refPrice + priceRange)));
		BOOST_TEST((candle.getClose() >= (refPrice - priceRange) && candle.getClose() <= (refPrice + priceRange)));
		BOOST_TEST((candle.getVolume() >= (refVol - volumeRange) && candle.getVolume() <= (refVol + volumeRange)));
	}
}

BOOST_AUTO_TEST_CASE(realTimeBars) {
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

	std::unordered_map<int, std::vector<std::unique_ptr<CandleStick>>> contracts;
	int i = 0;

	while (i <= 50) {
		//std::cout << i << std::endl;

		std::unique_lock<std::mutex> lock(mWrapper.getWrapperMutex());
		mWrapper.getWrapperConditional().wait(lock, [&] { return mWrapper.checkMockBufferFull(); });
		std::vector<std::unique_ptr<CandleStick>> temp = mWrapper.getProcessedFiveSecCandles();
		for (auto& i : temp) {
			contracts[i->getReqId()].push_back(std::move(i));
		}
		lock.unlock();

		i++;
	}

	mClient.cancelRealTimeBars();

	for (auto& it : contracts) {
		std::vector<std::unique_ptr<CandleStick>> temp = std::move(it.second);
		for (size_t i = 1; i < temp.size(); i++) {
			BOOST_TEST(temp[i]->getTime() - temp[i - 1]->getTime() == 5);
			BOOST_TEST(temp[i]->getHigh() >= temp[i]->getLow());
		}
	}
	
}

BOOST_AUTO_TEST_SUITE_END()