#include "OptionScanner.h"

OptionScanner::OptionScanner(const char* host, IBString ticker) : App(host), ticker(ticker), alertHandler() {
	// Request last quote for SPX upon class initiation to get closest option strikes
	SPX.symbol = ticker;
	SPX.secType = *SecType::IND;
	SPX.currency = "USD";
	SPX.primaryExchange = *Exchange::CBOE;

	// Update the date string at open
	std::time_t tmNow;
	tmNow = time(NULL);
	struct tm t = *localtime(&tmNow);

	todayDate = EndDateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

	// Create RTB request for SPX underlying **This will not be accessible until buffer is processed
	EC->reqRealTimeBars
	(1234
		, SPX
		, 5
		, *WhatToShow::TRADES
		, UseRTH::OnlyRegularTradingData
	);
}

//============================================================
// Open Market Data Processing Funtions
//============================================================

void OptionScanner::streamOptionData() {

	// Begin by requesting a market quote, and update strikes to send out requests
	EC->reqMktData(111, SPX, "", true);
	while (YW.getReqId() != 111) EC->checkMessages();

	updateStrikes(YW.lastTickPrice());

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

		// Use the wrapper conditional to check buffer
		std::unique_lock<std::mutex> lock(YW.wrapperMutex());
		YW.wrapperConditional().wait(lock, [&] { return YW.checkBufferFull(); });

		for (auto& candle : YW.processedFiveSecCandles()) {

			int req = candle->reqId();

			if (contracts.find(req) == contracts.end()) {
				std::shared_ptr<ContractData> cd = std::make_shared<ContractData>(req, std::move(candle));
				// registerAlertCallback(cd);
				contracts[req] = cd;
			}
			else {
				contracts[req]->updateData(std::move(candle));
			}
		}

		lock.unlock();

		// std::this_thread::sleep_for(std::chrono::seconds(5));

		// Every 1 minute, update the strikes
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::minutes elapsedTime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - lastExecutionTime);

		// Get the current time, and check with market close (3pm cst) to determine when to end the connection
		/*getDateTime();

		if (getDateVector()[0] == 15 && getDateVector()[5] >= 0) {
			prepareContractData();
			break;
		}*/

		if (elapsedTime >= interval) {
			updateStrikes(contracts[1234]->currentPrice());
			cout << "Buffer size currently at: " << YW.bufferCapacity() << endl;
			lastExecutionTime = currentTime; // Update the  last execution time
		}

	}
}

void OptionScanner::updateStrikes(double price) {
	
	std::lock_guard<std::mutex> lock(optScanMutex_);

	vector<int> strikes = populateStrikes(price);

	// Clear contractsInScope set each time strikes are updated to ensure newly populated strikes always are in scope
	contractsInScope.clear();

	for (auto i : strikes) {
		// If the contracts map doesn't already contain the strike, then a new one has come into scope
		if (contracts.find(i) == contracts.end()) {

			// Create new contracts if not in map and add to queue for requests
			Contract con;
			con.symbol = ticker;
			con.secType = *SecType::OPT;
			con.currency = "USD";
			con.exchange = *Exchange::IB_SMART;
			con.primaryExchange = *Exchange::CBOE;
			con.lastTradeDateOrContractMonth = todayDate;
			con.strike = i;
			con.right = *ContractRight::CALL;

			// Insert into queue
			contractReqQueue.push(con);

			// Now change to put and insert
			con.right = *ContractRight::PUT;
			contractReqQueue.push(con);
		}

		// Update the contracts in the scope
		contractsInScope.insert(i);
		contractsInScope.insert(i + 1);
	}

	// Empty queue and create the requests
	while (!contractReqQueue.empty()) {
		Contract con = contractReqQueue.front();

		int req = 0; // Ends in 0 or 5 if call, 1 or 6 for puts
		if (con.right == *ContractRight::CALL) req = static_cast<int>(con.strike);
		if (con.right == *ContractRight::PUT) req = static_cast<int>(con.strike + 1);

		// Now create the request
		EC->reqRealTimeBars
		(req
			, con
			, 5
			, *WhatToShow::TRADES
			, UseRTH::OnlyRegularTradingData
		);

		// Update contract request vector
		addedContracts.push_back(req);

		contractReqQueue.pop();
	}

	// If addedContracts vector exceeds current buffer size, update the buffer
	if (static_cast<int>(addedContracts.size()) > YW.bufferCapacity()) YW.setBufferCapacity(static_cast<int>(addedContracts.size()));
	//OPTIONSCANNER_DEBUG("Buffer capacity updated. Now at {}", YW.candleBuffer.checkBufferCapacity());
}

//===================================================
// Alert Callback Functions
//===================================================

//void OptionScanner::registerAlertCallback(ContractData* cd) {
//	cd->registerAlert([this, cd](int data, const StandardDeviation& sdPrice, const StandardDeviation sdVol, const Candle c) {
//
//		// Make sure contract is in scope
//		if (contractsInScope.find(c.reqId) != contractsInScope.end()) {
//			vector<bool> v1 = cd->getHighLowComparisons();
//			vector<bool> v2 = SPXBars->getHighLowComparisons();
//			int compCode = Alerts::getComparisonCode(v1, v2);
//
//			StandardDeviation uSdPrice;
//			StandardDeviation uSdVol;
//			Candle uBars;
//
//			// Determine which timeframe of underlying data to send
//			switch (data)
//			{
//			case 1001:
//				uSdPrice = SPXBars->get5SecStDev().first;
//				uSdVol = SPXBars->get5SecStDev().second;
//				uBars = SPXBars->getFiveSecData().back();
//				break;
//			case 1002:
//				uSdPrice = SPXBars->get30SecStDev().first;
//				uSdVol = SPXBars->get30SecStDev().second;
//				uBars = SPXBars->getThirtySecData().back();
//				break;
//			case 1003:
//				uSdPrice = SPXBars->get1MinStDev().first;
//				uSdVol = SPXBars->get1MinStDev().second;
//				uBars = SPXBars->getOneMinData().back();
//				break;
//			case 1004:
//				uSdPrice = SPXBars->get5MinStDev().first;
//				uSdVol = SPXBars->get5MinStDev().second;
//				uBars = SPXBars->getFiveMinData().back();
//				break;
//			}
//
//			Alerts::AlertData* a = new Alerts::AlertData(c, data, sdVol, sdPrice, uSdVol, uSdPrice, uBars, compCode);
//			alertHandler.inputAlert(a);
//		}
//	});
//}

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

//=====================================================
// Helper Functions
//=====================================================

vector<int> populateStrikes(double price) {
	vector<int> strikes;

	int multiple = 5; // SPX Strikes are in increments of 5
	// Round the price down to nearest increment
	int roundedPrice = int(price + (multiple / 2));
	roundedPrice -= roundedPrice % multiple;
	int strikePrice = roundedPrice - (multiple * 4);

	// This will give us 9 strikes in total
	while (strikePrice <= roundedPrice + (multiple * 4)) {
		strikes.push_back(strikePrice);
		strikePrice += multiple;
	}

	for (auto i : strikes) cout << i << " ";
	cout << endl;

	return strikes;
}

//===================================================
// Debuging and Test Output
//===================================================

//void OptionScanner::outputChain() {
//	// This will be an attempt to simulate a dynamic terminal
//
//	// Get underlying output
//	Candle c = SPXBars->getFiveSecData().back();
//	cout << "SPX Prices | open: " << c.open << " | high: " << c.high << " | low: " << c.low << " | close: " << c.close << endl;
//
//	cout << "============================================================================" << endl;
//	cout << "===========================Options Chain for SPX============================" << endl;
//	cout << "=========(CALLS)===========================================(PUTS)===========" << endl;
//	cout << "============================================================================" << endl;
//	cout << " Open  | High  |  Low  | Close    |Strike|    Open  | High  |  Low  | Close " << endl;
//	cout << "============================================================================" << endl;
//
//	for (auto i : sortedContractStrikes) {
//		Candle call(contracts[i]->getFiveSecData().back());
//		Candle put(contracts[i + 1]->getFiveSecData().back());
//
//		cout << "  " << call.open << "  |  " << call.high << "  |  " << call.low << "  |  " << call.close << "   |  " << i << "   |  "
//			<< put.open << "  |  " << put.high << "  |  " << put.low << "  |  " << put.close << "    " << endl;
//
//		cout << "============================================================================" << endl;
//	}
//
//	cout << endl;
//}