#define _CRT_SECURE_NO_WARNINGS

#include "../pch.h"

#include "../MockClasses/MockClient.h"
#include "../MockClasses/MockWrapper.h"
#include "../MockClasses/MockOptionScanner.h"
#include "ContractData.h"

#include <thread>

using namespace testing;


// Test the generation of 19 different contract requests based on the underlying price
TEST(MockOptionScannerTests, OptionChainGenerationTest) {
	// Set random generator to generate every 100 miliseconds
	MockOptionScanner mos(100);

	std::thread t([&] {
		mos.streamOptionData();
		});

	// New SPX price is used to update strikes every 100 miliseconds, so only sleep a short amount of time
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	mos.YW.setmDone(true);
	t.join();

	EXPECT_EQ(19, mos.checkContractMap());
}

// At each spx strikes update interval, ensure the spx price is past the previous range when it adds new contracts to the map
TEST(MockOptionScannerTests, SPXPriceUpdateTest) {
	MockOptionScanner mos(100);

	std::thread t([&] {
		mos.streamOptionData();
		});

	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	EXPECT_EQ(mos.currentSPX, mos.YW.getSPXPrice());

	// Loop for 5 times. On each loop, ensure the difference between the current SPX price and the new price
	// matches the discrepancy between the current contracts container size and the previous one

	double prev = mos.currentSPX;

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	EXPECT_FALSE(prev == mos.currentSPX);

	mos.YW.setmDone(true);
	t.join();
}