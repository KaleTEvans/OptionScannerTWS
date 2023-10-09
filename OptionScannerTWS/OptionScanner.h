
//===============================================================================
// Option scanner will set up requests for all contract data residing around a
// single stock, or in this case the SPX index. It will continously update all
// connected contracts throughout the day, and receive and monitor alerts. At
// the end of the day, OptionScanner will also be responsible for packaging and
// sending all contract data to the db
//===============================================================================
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include "App.h"
#include "ContractData.h"
#include "AlertHandler.h"
#include "DatabaseManager.h"

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <thread>
#include <algorithm>


class OptionScanner : public App {
public:
	// Option scanner will share the same constructor and destructor as App
	OptionScanner(const char* host, IBString ticker);

	// Run EC->checkMessages on its own thread
	void checkClientMessages();

	// Called each market open
	// void prepForOpen();
	
	// This function will use several functions provided in App to begin streaming contract data
	void streamOptionData();

	// Alert Callback Functions
	void registerAlertCallback(std::shared_ptr<ContractData> cd);

	// Functions for storing data after market close
	void prepareContractData();

	// Accessors
	std::condition_variable& optScanCV();
	std::mutex& optScanMtx();
	bool strikesUpdated();
	void changeStrikesUpdated();
	void outputChainData();

private:
	Contract SPX; // Contract to be monitored
	IBString ticker;

	IBString todayDate; // Updated each day

	std::thread messageThread_; // Used to continuously check messages
	bool closeEClienthread{ false };
	bool pauseMessages{ false };
	
	// We will update the strikes periodically to ensure that they are close to the underlying
	void updateStrikes(double price);
	bool strikesUpdated_{ false };

	// Debugging
	// This will output the options chain to the screen for debugging purposes
	// void outputChain();

	// This map will hold all of the contracts and will be updated repeatedly
	std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> contractChain_;

	std::queue<Contract> contractReqQueue; // Holds new contracts to request data
	std::unordered_set<int> contractsInScope; // If a contract isn't in the main scope of 18, it won't create an alert
	vector<int> addedContracts; // Keep track of all currently requested contracts

	std::unique_ptr<Alerts::AlertHandler> alertHandler;

	std::mutex optScanMutex_;
	std::condition_variable optScanCV_;
};

vector<int> populateStrikes(double price); // Helper function to return vector of strikes
