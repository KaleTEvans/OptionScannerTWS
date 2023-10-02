#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include "MockWrapper.h"
#include "MockClient.h"
#include "ContractData.h"
#include "MockSecurityReqHandler.h"

#include <queue>

// Alert Container for tests
struct Alert {
	TimeFrame tf;
	std::shared_ptr<ContractData> cd;
	std::shared_ptr<ContractData> SPX;
	std::shared_ptr<Candle> candle;
};

class MockOptionScanner {
public:
	// Option scanner will share the same constructor and destructor as App
	MockOptionScanner(int delay);

	int checkContractMap();

	// This function will use several functions provided in App to begin streaming contract data
	void streamOptionData();

	// Alert Callback Functions
	void registerAlertCallback(std::shared_ptr<ContractData> cd);

	// Functions for storing data after market close
	// void prepareContractData();

	// Test accessors
	double currentSPX();
	int diffChainSize();
	int prevBufferCapacity();
	std::unordered_set<int> finalContractCt();

	MockClient EC;
	MockWrapper YW;

	bool strikesUpdated{ false };
	std::mutex optScanMutex_;
	std::condition_variable mosCnditional;

	std::queue<Alert> alertQueue;

private:
	Contract SPX; // Contract to be monitored
	Securities::SecurityRequestHandler SPX_;

	IBString ticker;

	int delay_; // Set the delay for client candle generation

	IBString todayDate; // Updated each day

	double currentSPX_{ 0 }; // Monitor SPX price to ensure that when the contract map is updated it is because it is out of range
	double prevSPX_{ 0 };
	int curChainSize_{ 0 };
	int prevChainSize_{ 0 };
	int prevBufferCapacity_{ 0 };

	// We will update the strikes periodically to ensure that they are close to the underlying
	void updateStrikes(double price);

	std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> contractChain_;

	std::queue<Contract> contractReqQueue; // Holds new contracts to request data
	std::unordered_set<int> contractsInScope; // If a contract isn't in the main scope of 18, it won't create an alert
	std::unordered_set<int> addedContracts; // Keep track of all currently requested contracts

	// Alerts::AlertHandler alertHandler;
};

vector<int> populateStrikes(double price); // Helper function to return vector of strikes

