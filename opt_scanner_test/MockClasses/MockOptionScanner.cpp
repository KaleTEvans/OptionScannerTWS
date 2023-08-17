//#include "pch.h"
#include "MockOptionScanner.h"

MockOptionScanner::MockOptionScanner(int delay) : delay_(delay), EC(YW) {

	// Request last quote for SPX upon class initiation to get closest option strikes
	SPX.symbol = "SPX";
	SPX.secType = "IND";
	SPX.currency = "USD";

	EC.setCandleInterval(delay);

	// Create RTB request for SPX underlying **This will not be accessible until buffer is processed
	EC.reqRealTimeBars
	(1234
		, SPX
		, 0
		, ""
		, true
	);
}

// Test function
int MockOptionScanner::checkContractMap() { return contracts.size(); }

//============================================================
// Open Market Data Processing Funtions
//============================================================

void MockOptionScanner::streamOptionData() {

	updateStrikes(YW.getSPXPrice());
	currentSPX = YW.getSPXPrice();

	// This functionality will keep track of the time in order to update the strikes periodically
	constexpr int intervalMiliSeconds = 100;
	const std::chrono::milliseconds interval(intervalMiliSeconds);

	std::chrono::steady_clock::time_point lastExecutionTime = std::chrono::steady_clock::now();


	//==========================================================================
	// This while loop is important, as it will be open the entire day, and 
	// collect all options data and be responsible for sending alerts and all
	// other info
	//==========================================================================

	while (YW.notDone()) {

		// Use the wrapper conditional to check buffer
		std::unique_lock<std::mutex> lock(YW.getWrapperMutex());
		YW.getWrapperConditional().wait(lock, [&] { return YW.checkMockBufferFull(); });

		for (auto& candle : YW.getProcessedFiveSecCandles()) {

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

		// Update currentSPX
		currentSPX = contracts[1234]->currentPrice();

		lock.unlock();

		// std::this_thread::sleep_for(std::chrono::seconds(5));

		// Every 1 minute, update the strikes
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::milliseconds elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastExecutionTime);

		// Get the current time, and check with market close (3pm cst) to determine when to end the connection
		/*getDateTime();

		if (getDateVector()[0] == 15 && getDateVector()[5] >= 0) {
			prepareContractData();
			break;
		}*/

		if (elapsedTime >= interval) {
			updateStrikes(contracts[1234]->currentPrice());
			std::cout << "Buffer size currently at: " << YW.getBufferCapacity() << std::endl;
			lastExecutionTime = currentTime; // Update the  last execution time
		}

	}

	if (!YW.notDone()) {
		std::cout << "Wrapper notified done. Terminating." << std::endl;
		EC.cancelRealTimeBars();
	}
}

void MockOptionScanner::updateStrikes(double price) {

	std::lock_guard<std::mutex> lock(optScanMutex_);

	vector<int> strikes = populateStrikes(price);

	// Clear contractsInScope set each time strikes are updated to ensure newly populated strikes always are in scope
	contractsInScope.clear();

	for (auto i : strikes) {
		// If the contracts map doesn't already contain the strike, then a new one has come into scope
		if (contracts.find(i) == contracts.end()) {

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

	//if (contractReqQueue.size() > 0 && currentSPX != YW.getSPXPrice()) currentSPX = contracts[1234]->currentPrice();

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
		addedContracts.push_back(req);

		contractReqQueue.pop();
	}

	// If addedContracts vector exceeds current buffer size, update the buffer
	if (static_cast<int>(addedContracts.size()) > YW.getBufferCapacity()) YW.setBufferCapacity(static_cast<int>(addedContracts.size()));
	//OPTIONSCANNER_DEBUG("Buffer capacity updated. Now at {}", YW.candleBuffer.checkBufferCapacity());
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

	for (auto i : strikes) std::cout << i << " ";
	std::cout << std::endl;

	return strikes;
}
