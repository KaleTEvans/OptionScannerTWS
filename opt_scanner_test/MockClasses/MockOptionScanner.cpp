//#include "pch.h"
#include "MockOptionScanner.h"

MockOptionScanner::MockOptionScanner(int delay) : delay_(delay), EC(YW), SPX_("SPX", 1234, 5, 4, "IND") {

	// Request last quote for SPX upon class initiation to get closest option strikes
	/*SPX.symbol = "SPX";
	SPX.secType = "IND";
	SPX.currency = "USD";*/

	EC.setCandleInterval(delay);

	// Create RTB request for SPX underlying **This will not be accessible until buffer is processed
	/*EC.reqRealTimeBars
	(1234
		, SPX
		, 0
		, ""
		, true
	);*/

	// Add SPX to the added contracts set
	//addedContracts.insert(1234);

	SPX_.initializeOptionRequests(EC, 111);

	// Initialzie the contract chain
	contractChain_ = std::make_shared<std::unordered_map<int, std::shared_ptr<ContractData>>>();
}

// Test function Accessors
int MockOptionScanner::checkContractMap() { return contractChain_->size(); }
double MockOptionScanner::currentSPX() { return (currentSPX_); }
int MockOptionScanner::diffChainSize() { return (curChainSize_ - prevChainSize_); }
int MockOptionScanner::prevBufferCapacity() { return prevBufferCapacity_; }
std::unordered_set<int> MockOptionScanner::finalContractCt() { return addedContracts; }


//============================================================
// Open Market Data Processing Funtions
//============================================================

void MockOptionScanner::streamOptionData() {

	currentSPX_ = YW.getSPXPrice();
	//updateStrikes(currentSPX_);
	SPX_.updateOptionRequests(EC, currentSPX_, "", contractChain_);

	// Add temporary comparison value for Chain
	prevChainSize_ = 19;


	//==========================================================================
	// This while loop is important, as it will be open the entire day, and 
	// collect all options data and be responsible for sending alerts and all
	// other info
	//==========================================================================

	while (YW.notDone()) {

		// Use the wrapper conditional to check buffer
		std::unique_lock<std::mutex> lock(YW.getWrapperMutex());
		YW.getWrapperConditional().wait(lock, [&] { return YW.checkMockBufferFull(); });

		// Update test variables
		prevBufferCapacity_ = YW.getBufferCapacity();

		for (auto& candle : YW.getProcessedFiveSecCandles()) {

			int req = candle->reqId();

			if (contractChain_->find(req) != contractChain_->end()) {
				contractChain_->at(req)->updateData(std::move(candle));
				
			}
			else {
				std::shared_ptr<ContractData> cd = std::make_shared<ContractData>(req, std::move(candle));
				registerAlertCallback(cd);
				contractChain_->insert({ req, cd });
			}
		}

		// Update test variables
		curChainSize_ = contractChain_->size();
		currentSPX_ = contractChain_->at(1234)->currentPrice();
		// std::cout << "Buffer size currently at: " << YW.getBufferCapacity() << std::endl;

		lock.unlock();
		mosCnditional.notify_one();

		//updateStrikes(contractChain_->at(1234)->currentPrice());
		SPX_.updateOptionRequests(EC, contractChain_->at(1234)->currentPrice(), "", contractChain_);
		strikesUpdated = true;
	}

	if (!YW.notDone()) {
		std::cout << "Wrapper notified done. Terminating." << std::endl;
		EC.cancelRealTimeBars();
	}
}

//==============================================================
// Alert Functions
//==============================================================

void MockOptionScanner::registerAlertCallback(std::shared_ptr<ContractData> cd) {
	cd->registerAlert([this, cd](TimeFrame tf, std::shared_ptr<Candle> candle) {
		
		// For the purpose of the mock testing, we will just add these instances to a queue
		Alert a;
		a.tf = tf;
		a.cd = cd;
		a.SPX = contractChain_->at(1234);
		a.candle = candle;

		alertQueue.push(a);
	});
}

//==============================================================
// Option Chain Update Helper Functions
//==============================================================

void MockOptionScanner::updateStrikes(double price) {

	std::lock_guard<std::mutex> lock(optScanMutex_);

	vector<int> strikes = populateStrikes(price);

	// Clear contractsInScope set each time strikes are updated to ensure newly populated strikes always are in scope
	contractsInScope.clear();

	for (auto i : strikes) {
		// If the contracts map doesn't already contain the strike, then a new one has come into scope
		if (contractChain_->find(i) == contractChain_->end()) {

			// Create new contracts if not in map and add to queue for requests
			Contract con;
			con.symbol = "SPX";
			con.secType = "OPT";
			con.currency = "USD";
			con.strike = i;
			con.right = "CALL";

			// Insert into queue
			contractReqQueue.push(con);

			// Now change to put and insert
			con.right = "PUT";
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
		if (con.right == "CALL") req = static_cast<int>(con.strike);
		if (con.right == "PUT") req = static_cast<int>(con.strike + 1);

		// std::cout << "Popping from queue Req: " << req << std::endl;

		// Now create the request
		EC.reqRealTimeBars
		(req
			, con
			, 0
			, ""
			, true
		);

		// Update contract request vector
		addedContracts.insert(req);

		contractReqQueue.pop();
	}

	// If addedContracts vector exceeds current buffer size, update the buffer
	if (static_cast<int>(addedContracts.size()) > YW.getBufferCapacity()) YW.setBufferCapacity(static_cast<int>(addedContracts.size()));
	//OPTIONSCANNER_DEBUG("Buffer capacity updated. Now at {}", YW.candleBuffer.checkBufferCapacity());
	strikesUpdated = true;
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

	// for (auto i : strikes) std::cout << i << " ";
	// std::cout << std::endl;

	return strikes;
}
