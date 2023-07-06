
//===============================================================================
// Option scanner will set up requests for all contract data residing around a
// single stock, or in this case the SPX index. It will continously update all
// connected contracts throughout the day, and receive and monitor alerts. At
// the end of the day, OptionScanner will also be responsible for packaging and
// sending all contract data to the db
//===============================================================================

#pragma once

#include "App.h"
#include "ContractData.h"
#include "AlertHandler.h"

#include <unordered_map>
#include <chrono>
#include <thread>
#include <algorithm>


class OptionScanner : public App {
public:
	// Option scanner will share the same constructor and destructor as App
	OptionScanner(const char* host, IBString ticker);
	
	// This function will use several functions provided in App to begin streaming contract data
	void streamOptionData();

	// Alert Callback Functions
	void registerAlertCallback(ContractData * cd);
	// void showAlertOutput(int data, double stDevVol, double stDevPrice, Candle c);

	// Functions for storing data after market close
	void prepareContractData();

private:
	// We will update the strikes periodically to ensure that they are close to the underlying
	void updateStrikes();

	// Debugging
	// This will output the options chain to the screen for debugging purposes
	void outputChain();

	// This map will hold all of the contracts and will be updated repeatedly
	std::unordered_map<int, ContractData*> contracts;

	// Contains data for the underlying
	ContractData* SPXBars = nullptr;

	// Historical data requests need to be incremented
	int historicalReq = 8000; // 8000 to avoid conflict with SPX price, be sure to update next bull market

	vector<int> optionStrikes;
	vector<int> sortedContractStrikes;

	Alerts::AlertHandler ah;
};

