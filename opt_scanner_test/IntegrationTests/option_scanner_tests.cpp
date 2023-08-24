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
	MockOptionScanner mos(25);

	std::thread t([&] {
		mos.streamOptionData();
		});

	// New SPX price is used to update strikes every 100 miliseconds, so only sleep a short amount of time
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	mos.YW.setmDone(true);
	t.join();

	EXPECT_EQ(19, mos.checkContractMap());
}

// Output test with no return value to ensure updates are made with mutexes and condition variables in place
TEST(MockOptionScannerTests, SPXPriceUpdateTest) {
	MockOptionScanner mos(25);

	std::thread t([&] {
		mos.streamOptionData();
		});

	for (size_t i = 0; i < 10; i++) {
		// Lock the option scanner mutex to wait for updated data
		std::unique_lock<std::mutex> lock(mos.optScanMutex_);
		mos.mosCnditional.wait(lock, [&] { return mos.strikesUpdated; });

		std::cout << "Contract Map Size: " << mos.checkContractMap() << std::endl;
		std::cout << "Diff Chain Size: " << mos.diffChainSize() << std::endl;
		std::cout << "Diff SPX: " << mos.diffSPX() << std::endl;

		// Reset strikesUpdated and wait for next update
		mos.strikesUpdated = false;
		lock.unlock();
	}


	mos.YW.setmDone(true);
	t.join();
}