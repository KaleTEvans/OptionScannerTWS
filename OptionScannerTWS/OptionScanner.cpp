#include "OptionScanner.h"

OptionScanner::OptionScanner(const char* host, IBString ticker) : App(host, ticker) {
	// Start a request for realtime bars for the underlying
	EC->reqRealTimeBars
	(1234
		, getUnderlyingContract()
		, 5
		, *WhatToShow::TRADES
		, UseRTH::OnlyRegularTradingData
	);

	while (YW.underlyingRTBs.reqId != 1234) EC->checkMessages();
	SPXBars = new ContractData(YW.underlyingRTBs.reqId, YW.underlyingRTBs, true);
}

//============================================================
// Open Market Data Processing Funtions
//============================================================

void OptionScanner::streamOptionData() {

	updateStrikes();

	// This functionality will keep track of the time in order to update the strikes periodically
	constexpr int intervalMinutes = 1;
	const std::chrono::minutes interval(intervalMinutes);

	std::chrono::steady_clock::time_point lastExecutionTime = std::chrono::steady_clock::now();


	//==========================================================================
	// This while loop is important, as it will be open the entire day, and 
	// collect all options data and be responsible for sending alerts and all
	// other info
	//==========================================================================

	while (YW.notDone()) {
		EC->checkMessages();

		// Update underlying
		if (YW.underlyingRTBs.reqId == 1234) SPXBars->updateData(YW.underlyingRTBs);

		if (!YW.fiveSecCandles.empty()) {
			for (auto i : YW.fiveSecCandles) {
				if (contracts.find(i.reqId) == contracts.end()) {
					ContractData* cd = new ContractData(i.reqId, i);
					registerAlertCallback(cd);
					contracts[i.reqId] = cd;

				}
				else {
					contracts[i.reqId]->updateData(i);
				}
			}

			std::this_thread::sleep_for(std::chrono::seconds(5));
		}

		// Every 1 minute, update the strikes
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::minutes elapsedTime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - lastExecutionTime);

		// Get the current time, and check with market close (3pm cst) to determine when to end the connection
		getDateTime();

		if (getDateVector()[0] == 15 && getDateVector()[5] >= 0) {
			prepareContractData();
			break;
		}

		if (elapsedTime >= interval) {
			updateStrikes();
			cout << "Buffer size currently at: " << YW.candleBuffer.buffer.size() << endl;
			lastExecutionTime = currentTime; // Update the  last execution time
		}

	}
}

void OptionScanner::updateStrikes() {
	// Clear out the strikes vector for each use
	optionStrikes.clear();

	populateStrikes(5, historicalReq);
	historicalReq++;

	cout << "Strikes populated" << endl;

	for (auto i : strikes) {
		// If the contracts map doesn't already contain the strike, then a new one has come into scope
		if (contracts.find(i) == contracts.end()) {
			optionStrikes.push_back(i);
			// Contracts with reqId +1 will represent put options
			optionStrikes.push_back(i + 1);
		}
	}


	cout << "Option Strikes: ";
	for (auto i : optionStrikes) cout << i << " ";
	cout << endl;

	// Add the strikes to the sorted array
	for (auto i : optionStrikes) if (i % 5 == 0) sortedContractStrikes.push_back(i);
	std::sort(sortedContractStrikes.begin(), sortedContractStrikes.end());

	// Be sure to update the size of the buffer capaciity in the wrapper if more strikes are added
	if (sortedContractStrikes.size() > 18) {
		YW.candleBuffer.capacity = (sortedContractStrikes.size());
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

		con.lastTradeDateOrContractMonth = EndDateTime(date[2], date[1], date[0]);

		// These next variables depend on whether or not a put or call
		if (i % 5 == 0) {
			con.right = *ContractRight::CALL;
			con.strike = i;
		}
		else {
			con.right = *ContractRight::PUT;
			con.strike = double(i) - 1;
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

//===================================================
// Alert Callback Functions
//===================================================

void OptionScanner::registerAlertCallback(ContractData* cd) {
	cd->registerAlert([this, cd](int data, StandardDeviation& sdVol, StandardDeviation& sdPrice, Candle c) {

		vector<bool> v1 = cd->getHighLowComparisons();
		vector<bool> v2 = SPXBars->getHighLowComparisons();
		int compCode = Alerts::getComparisonCode(v1, v2);

		Alerts::AlertData a(c, data, sdVol, sdPrice, cd->getDailyHigh(), cd->getDailyLow(), 
			cd->getCumulativeVol(), SPXBars->getCurrentPrice(), compCode);
		ah.outputAlert(a);
		// showAlertOutput(data, stDevVol, stDevPrice, c);
		});
}

//void OptionScanner::showAlertOutput(int data, double stDevVol, double stDevPrice, Candle c) {
//	cout << "Callback Received for contract: " << c.reqId << " Code: " << data << " volume: " << c.volume << " close price: " << c.close << endl;
//}


//===================================================
// Post Close Data Processing
//===================================================

void OptionScanner::prepareContractData() {
	std::cout << "Market closed, ending realTimeBar connection" << std::endl;
	
	for (auto i : contracts) EC->cancelRealTimeBars(i.first);
	EC->cancelRealTimeBars(1234);
}


//===================================================
// Debuging and Test Output
//===================================================

void OptionScanner::outputChain() {
	// This will be an attempt to simulate a dynamic terminal

	// Get underlying output
	Candle c = SPXBars->getFiveSecData().back();
	cout << "SPX Prices | open: " << c.open << " | high: " << c.high << " | low: " << c.low << " | close: " << c.close << endl;

	cout << "============================================================================" << endl;
	cout << "===========================Options Chain for SPX============================" << endl;
	cout << "=========(CALLS)===========================================(PUTS)===========" << endl;
	cout << "============================================================================" << endl;
	cout << " Open  | High  |  Low  | Close    |Strike|    Open  | High  |  Low  | Close " << endl;
	cout << "============================================================================" << endl;

	for (auto i : sortedContractStrikes) {
		Candle call(contracts[i]->getFiveSecData().back());
		Candle put(contracts[i + 1]->getFiveSecData().back());

		cout << "  " << call.open << "  |  " << call.high << "  |  " << call.low << "  |  " << call.close << "   |  " << i << "   |  "
			<< put.open << "  |  " << put.high << "  |  " << put.low << "  |  " << put.close << "    " << endl;

		cout << "============================================================================" << endl;
	}

	cout << endl;
}