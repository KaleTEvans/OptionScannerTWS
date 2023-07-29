//#define _CRT_SECURE_NO_WARNINGS
//
#define BOOST_TEST_MODULE client_wrapper_test
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
//#include <boost/test/data/monomorphic.hpp>
//#include <boost/test/tools/assertion_result.hpp>
//#include <boost/test/tools/floating_point_comparison.hpp>

#include "MockClient.h"

namespace data = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(ClientWrapperTests)

BOOST_AUTO_TEST_CASE(currentTime) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	auto currentTime = std::chrono::system_clock::now().time_since_epoch();
	std::time_t unixTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
	long time = static_cast<long>(unixTime);

	mClient.reqCurrentTime();

	BOOST_REQUIRE_EQUAL(mWrapper.time_, time);
}

const auto test_data = data::make({
	std::make_tuple("20230705 09:30:00", "3600 S", "5 secs"),
	std::make_tuple("20230705 08:30:00", "21600 S", "30 secs"),
	std::make_tuple("20230705 08:30:00", "43200 S", "60 secs")
});

BOOST_DATA_TEST_CASE(historicalData, test_data, endDateTime, durationStr, barSizeSetting) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);

	Contract con;
	con.symbol = "SPX";
	con.secType = "IND";

	mClient.reqHistoricalData(100, con, endDateTime, durationStr, barSizeSetting, "", 0, 0, false);

	BOOST_CHECK_EQUAL(mWrapper.getHistoricCandles().size(), 720);
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

		BOOST_CHECK(candle.getOpen() >= (spxRefPrice - priceRange) && candle.getOpen() <= (spxRefPrice + priceRange));
		BOOST_CHECK(candle.getHigh() >= (spxRefPrice - priceRange) && candle.getHigh() <= (spxRefPrice + priceRange));
		BOOST_CHECK(candle.getLow() >= (spxRefPrice - priceRange) && candle.getLow() <= (spxRefPrice + priceRange));
		BOOST_CHECK(candle.getClose() >= (spxRefPrice - priceRange) && candle.getClose() <= (spxRefPrice + priceRange));
		BOOST_CHECK(candle.getVolume() >= (spxRefVol - volumeRange) && candle.getVolume() <= (spxRefVol + volumeRange));
	}
}

BOOST_AUTO_TEST_CASE(fiveSecCandles) {
	std::cout << calculateOptionPrice(4575, 4581) << std::endl;
	std::cout << calculateOptionPrice(4576, 4581) << std::endl;
	std::cout << calculateOptionPrice(4580, 4581) << std::endl;
	std::cout << calculateOptionPrice(4581, 4580) << std::endl;
	std::cout << calculateOptionPrice(4585, 4581) << std::endl;
	std::cout << calculateOptionPrice(4586, 4581) << std::endl;
	std::cout << calculateOptionPrice(4590, 4581) << std::endl;
	std::cout << calculateOptionPrice(4591, 4581) << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()