#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include "MockWrapper.h"
#include "MockClient.h"
#include "ContractData.h"

class MockOptionScanner {
public:
	// Option scanner will share the same constructor and destructor as App
	MockOptionScanner(int delay);

	int checkContractMap();

	// This function will use several functions provided in App to begin streaming contract data
	void streamOptionData();

	// Alert Callback Functions
	// void registerAlertCallback(std::shared_ptr<ContractData> cd);

	// Functions for storing data after market close
	// void prepareContractData();

	double currentSPX; // Monitor SPX price to ensure that when the contract map is updated it is because it is out of range

	MockClient EC;
	MockWrapper YW;

private:
	Contract SPX; // Contract to be monitored
	IBString ticker;

	int delay_; // Set the delay for client candle generation

	IBString todayDate; // Updated each day

	// We will update the strikes periodically to ensure that they are close to the underlying
	void updateStrikes(double price);

	// This map will hold all of the contracts and will be updated repeatedly
	std::unordered_map<int, std::shared_ptr<ContractData>> contracts;

	std::queue<Contract> contractReqQueue; // Holds new contracts to request data
	std::unordered_set<int> contractsInScope; // If a contract isn't in the main scope of 18, it won't create an alert
	std::vector<int> addedContracts; // Keep track of all currently requested contracts

	// Alerts::AlertHandler alertHandler;

	std::mutex optScanMutex_;
};

vector<int> populateStrikes(double price); // Helper function to return vector of strikes

