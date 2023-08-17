#define _CRT_SECURE_NO_WARNINGS

#include "../pch.h"

#include "../MockClasses/MockWrapper.h"
#include "../MockClasses/MockClient.h"

#include <chrono>

using namespace testing;

// Test different completion times for various methods
TEST(BenchmarkTests, candleStickRetreival) {
	MockWrapper mWrapper;
	MockClient mClient(mWrapper);
	//mWrapper.showHistoricalDataOutput();

	Contract con;
	con.symbol = "SPX";
	con.secType = "IND";

	using Clock = std::chrono::high_resolution_clock;
	using Second = std::chrono::duration<double, std::ratio<1> >;

	std::vector<std::unique_ptr<Candle>> ptrSamples;
	std::vector<Candle> copySamples;

	std::chrono::time_point<Clock> m_beg{ Clock::now() };

	// Benchmark test will determine the shortest time for each data structure and method
	// to retrieve historical data, and move items to a new vector to simulate other classes using the wrapper

	// Warm up by creating a few requests
	mWrapper.candleBenchmarkSwitch = 1;
	mClient.reqHistoricalData(100, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);
	auto tempV = mWrapper.getHistoricCandles();
	tempV.clear();

	mWrapper.candleBenchmarkSwitch = 2;
	mClient.reqHistoricalData(100, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);
	auto tempQ = mWrapper.getHistoricCandles();
	tempQ.clear();

	mWrapper.candleBenchmarkSwitch = 3;
	mClient.reqHistoricalData(100, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);
	tempV = mWrapper.getHistoricCandles();
	tempV.clear();

	mWrapper.candleBenchmarkSwitch = 4;
	mClient.reqHistoricalData(100, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);
	tempQ = mWrapper.getHistoricCandles();
	tempQ.clear();

	mWrapper.clearTestContainers();
	
	///////////////////////// Method 1 : Using a vector of smart pointers //////////////////
	mWrapper.candleBenchmarkSwitch = 1;
	m_beg = Clock::now();
	mClient.reqHistoricalData(100, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);
	std::vector<std::unique_ptr<Candle>> con1 = mWrapper.getHistoricCandles();

	for (size_t i = 0; i < con1.size(); i++) {
		ptrSamples.push_back(std::move(con1[i]));
	}

	auto elapsed1 = std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
	auto sample1 = ptrSamples.size();
	ptrSamples.clear();

	////////////////////////// Method 2 : Using a queue of smart pointers ////////////////////
	mWrapper.candleBenchmarkSwitch = 2;
	m_beg = Clock::now();
	mClient.reqHistoricalData(100, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);
	std::queue<std::unique_ptr<Candle>> con2 = mWrapper.getMovedCandleQueue();

	while (!con2.empty()) {
		ptrSamples.push_back(std::move(con2.front()));
		con2.pop();
	}

	auto elapsed2 = std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
	auto sample2 = ptrSamples.size();
	ptrSamples.clear();

	//////////////////////////// Method 3: Using a vector of copied candles //////////////////
	mWrapper.candleBenchmarkSwitch = 3;
	m_beg = Clock::now();
	mClient.reqHistoricalData(100, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);
	std::vector<Candle> con3 = mWrapper.getCopiedCandleVector();

	for (size_t i = 0; i < con3.size(); i++) {
		copySamples.push_back(con3[i]);
	}

	auto elapsed3 = std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
	auto sample3 = copySamples.size();
	copySamples.clear();

	//////////////////////////// Method 4: Using a queue of copied candles ////////////////////
	mWrapper.candleBenchmarkSwitch = 4;
	m_beg = Clock::now();
	mClient.reqHistoricalData(100, con, "20230705 09:30:00", "3600 S", "5 secs", "", 0, 0, false);
	std::queue<Candle> con4 = mWrapper.getCopiedCandleQueue();

	while (!con4.empty()) {
		copySamples.push_back(con4.front());
		con4.pop();
	}

	auto elapsed4 = std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
	auto sample4 = copySamples.size();
	copySamples.clear();

	std::cout << std::endl;
	std::cout << "Time elapsed Vector of Smart Pointers: " << elapsed1 << " Sample Size: " << sample1 << std::endl;
	std::cout << std::endl;
	std::cout << "Time elapsed Queue of Smart Pointers: " << elapsed2 << " Sample Size: " << sample2 << std::endl;
	std::cout << std::endl;
	std::cout << "Time elapsed Queue of Smart Pointers: " << elapsed3 << " Sample Size: " << sample3 << std::endl;
	std::cout << std::endl;
	std::cout << "Time elapsed Queue of Smart Pointers: " << elapsed4 << " Sample Size: " << sample4 << std::endl;
	std::cout << std::endl;
}

