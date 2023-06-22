#pragma once

#include "App.h"
#include "ContractData.h"

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

private:
	// We will update the strikes periodically to ensure that they are close to the underlying
	void updateStrikes();

	// Debugging
	// This will output the options chain to the screen for debugging purposes
	void outputChain();

	// This map will hold all of the contracts and will be updated repeatedly
	std::unordered_map<int, ContractData*> contracts;

	vector<int> optionStrikes;
	vector<int> sortedContractStrikes;
};