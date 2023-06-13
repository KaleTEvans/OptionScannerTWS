#include "OptionScanner.h"

OptionScanner::OptionScanner(const char* host, IBString ticker) : App(host, ticker) {}

void OptionScanner::streamOptionData() {

	updateStrikes();

	// This functionality will keep track of the time in order to update the strikes periodically
	constexpr int intervalMinutes = 10;
	const std::chrono::minutes interval(intervalMinutes);

	std::chrono::steady_clock::time_point lastExecutionTime = std::chrono::steady_clock::now();

	//==========================================================================
	// This while loop is important, as it will be open the entire day, and 
	// collect all options data and be responsible for sending alerts and all
	// other info
	//==========================================================================
	while (YW.notDone()) {
		EC->checkMessages();

		if (!YW.fiveSecCandles.empty()) {
			for (auto i : YW.fiveSecCandles) {
				if (contracts.find(i.reqId) == contracts.end()) {
					ContractData* cd = new ContractData(i.reqId, i);
					contracts[i.reqId] = cd;
				}
				else {
					contracts[i.reqId]->updateData(i);
				}
			}
		}

		// Every 10 minutes, update the strikes
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::minutes elapsedTime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - lastExecutionTime);

		if (elapsedTime >= interval) {
			updateStrikes();
			lastExecutionTime = currentTime; // Update the  last execution time
		}

		std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep for a short duration
	}
}

void OptionScanner::updateStrikes() {
	// Clear out the strikes vector for each use
	optionStrikes.clear();

	populateStrikes();

	for (auto i : strikes) {
		// If the contracts map doesn't already contain the strike, then a new one has come into scope
		if (contracts.find(i) == contracts.end()) {
			optionStrikes.push_back(i);
			// Contracts with reqId +1 will represent put options
			optionStrikes.push_back(i + 1);
		}
	}

	// Now we will create a request for each option strike not already in the map
	for (auto i : optionStrikes) {
		// Create the contract
		Contract con;
		con.symbol = getTicker();
		con.secType = *SecType::OPT;
		con.currency = "USD";
		con.exchange = *Exchange::IB_SMART;
		con.primaryExchange = *Exchange::CBOE;

		// Retrieve Date
		getDateTime();
		vector<int> date = getDateVector();

		con.expiry = EndDateTime(date[2], date[1], date[0]);

		// These next variables depend on whether or not a put or call
		if (!i % 5) {
			con.right = *ContractRight::CALL;
			con.strike = i;
		}
		else {
			con.right = *ContractRight::PUT;
			con.strike = i - 1;
		}

		// Now create the request
		EC->reqRealTimeBars
		(i
			, con
			, 5
			, *WhatToShow::TRADES
			, UseRTH::OnlyRegularTradingData
		);
	}
}