#include "OptionScanner.h"
#include "Logger.h"

OptionScanner::OptionScanner(const char* host, IBString ticker) : App(host), ticker(ticker) {

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

	dbm->setCandleTables();

	// Create RTB request for SPX underlying **This will not be accessible until buffer is processed
	// YW.showRealTimeDataOutput();
	EC->reqRealTimeBars
	(1234
		, SPX
		, 5
		, *WhatToShow::TRADES
		, UseRTH::OnlyRegularTradingData
	);

	addedContracts.push_back(1234);
	OPTIONSCANNER_DEBUG("Initializing scanner ... Request 1234 sent to client");

	// Initialzie the contract chain
	contractChain_ = std::make_shared<std::unordered_map<int, std::shared_ptr<ContractData>>>();

	// Initialize the alert handler with a pointer to the contract map
	alertHandler = std::make_unique<Alerts::AlertHandler>(contractChain_);

	// Start the checkMessages thread
	messageThread_ = std::thread(&OptionScanner::checkClientMessages, this);
}

void OptionScanner::checkClientMessages() {
	while (!closeEClienthread) {
		EC->checkMessages();
		if (pauseMessages) std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

//============================================================
// Open Market Data Processing Funtions
//============================================================

void OptionScanner::streamOptionData() {

	// Begin by requesting a market quote, and update strikes to send out requests
	SPX.exchange = *Exchange::IB_SMART;
	EC->reqMktData(111, SPX, "", true);

	// Wait for request
	while (YW.getReqId() != 111) continue;
	
	OPTIONSCANNER_INFO("Market data request received, last tick price: {}", YW.lastTickPrice());

	updateStrikes(YW.lastTickPrice());

	//==========================================================================
	// This while loop is important, as it will be open the entire day, and 
	// collect all options data and be responsible for sending alerts and all
	// other info
	//==========================================================================

	while (YW.notDone()) {

		// Use the wrapper conditional to check buffer
		std::unique_lock<std::mutex> lock(YW.wrapperMutex());
		YW.wrapperConditional().wait(lock, [&] { return YW.checkBufferFull(); });
		OPTIONSCANNER_DEBUG("Buffer full, current capacity: {}", YW.bufferCapacity());

		for (auto& candle : YW.processedFiveSecCandles()) {

			int req = candle->reqId();

			if (contractChain_->find(req) != contractChain_->end()) {
				contractChain_->at(req)->updateData(std::move(candle));
			}
			else {
				std::shared_ptr<ContractData> cd = std::make_shared<ContractData>(req, std::move(candle));

				// Add SQL connection to contractData
				cd->setupDatabaseManager(dbm);

				registerAlertCallback(cd);
				contractChain_->insert({ req, cd });
			}
		}

		strikesUpdated_ = true;

		lock.unlock();
		optScanCV_.notify_one();

		updateStrikes(contractChain_->at(1234)->currentPrice());
		OPTIONSCANNER_DEBUG("Strikes updated, current buffer capacity: {}", YW.bufferCapacity());
	}
}

// Accessors
std::condition_variable& OptionScanner::optScanCV() { return optScanCV_; }
std::mutex& OptionScanner::optScanMtx() { return optScanMutex_; }

bool OptionScanner::strikesUpdated() { return strikesUpdated_; }
void OptionScanner::changeStrikesUpdated() { strikesUpdated_ = false; }

void OptionScanner::outputChainData() {
	for (auto& i : *contractChain_) {
		std::cout << "Strike: " << i.first << " Price: " << i.second->currentPrice() << std::endl;
	}
}

void OptionScanner::updateStrikes(double price) {
	
	std::lock_guard<std::mutex> lock(optScanMutex_);

	vector<int> strikes = populateStrikes(price);

	// Clear contractsInScope set each time strikes are updated to ensure newly populated strikes always are in scope
	contractsInScope.clear();

	for (auto i : strikes) {
		// If the contracts map doesn't already contain the strike, then a new one has come into scope
		if (contractChain_->find(i) == contractChain_->end()) {

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

	if (!contractReqQueue.empty()) OPTIONSCANNER_INFO("{} new contracts added to request queue", contractReqQueue.size());

	////////////////////// 
	pauseMessages = true; // temporarily pause thread running checkMessages
	/////////////////////
	
	// Empty queue and create the requests
	while (!contractReqQueue.empty()) {
		Contract con = contractReqQueue.front();

		int req = 0; // Ends in 0 or 5 if call, 1 or 6 for puts
		if (con.right == *ContractRight::CALL) req = static_cast<int>(con.strike);
		else if (con.right == *ContractRight::PUT) req = static_cast<int>(con.strike + 1);

		// Now create the request
		EC->reqRealTimeBars
		(req
			, con
			, 5
			, *WhatToShow::TRADES
			, UseRTH::OnlyRegularTradingData
		);

		OPTIONSCANNER_DEBUG("Request {} sent to client", req);

		// Update contract request vector
		addedContracts.push_back(req);

		contractReqQueue.pop();
	}

	//////////////////////
	pauseMessages = false; // resume checking messages
	//////////////////////

	OPTIONSCANNER_DEBUG("New requests sent to queue, total option active requests: {}", addedContracts.size());

	// If addedContracts vector exceeds current buffer size, update the buffer
	if (static_cast<int>(addedContracts.size()) > YW.bufferCapacity()) {
		YW.setBufferCapacity(static_cast<int>(addedContracts.size()));
		OPTIONSCANNER_INFO("Current requests higher than buffer capacity... updating to size: {}", addedContracts.size());
	}
}

//===================================================
// Alert Callback Functions
//===================================================

void OptionScanner::registerAlertCallback(std::shared_ptr<ContractData> cd) {
	std::lock_guard<std::mutex> lock(optScanMutex_);
	cd->registerAlert([this, cd](TimeFrame tf, std::shared_ptr<Candle> candle) {

		// Make sure contract is in scope
		if (contractsInScope.find(candle->reqId()) != contractsInScope.end()) {

			alertHandler->inputAlert(tf, cd, contractChain_->at(1234), candle);
		}
	});
}


//===================================================
// Post Close Data Processing
//===================================================

void OptionScanner::prepareContractData() {
	std::cout << "Market closed, ending realTimeBar connection" << std::endl;
	
	for (auto& i : *contractChain_) EC->cancelRealTimeBars(i.first);
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
	int strikePrice = roundedPrice - (multiple * 5);

	// This will give us 9 strikes in total
	while (strikePrice <= roundedPrice + (multiple * 5)) {
		strikes.push_back(strikePrice);
		strikePrice += multiple;
	}

	//for (auto i : strikes) cout << i << " ";
	//cout << endl;

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