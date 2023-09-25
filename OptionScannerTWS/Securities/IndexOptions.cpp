#include "IndexOptions.h"

namespace Securities {

	Index::Index(IBString ticker, const int req, const int multiple, const int numStrikes) : 
		ticker_(ticker), indReq_(req), multiple_(multiple), numStrikes_(numStrikes) 
	{
		con_.symbol = ticker;
		con_.symbol = ticker;
		con_.secType = *SecType::IND;
		con_.currency = "USD";
		con_.primaryExchange = *Exchange::CBOE;
	}

	int Index::numReqs() const { return currentReqs_.size(); }
	std::vector<int> Index::currentActiveReqs() const { return currentReqs_; }
	bool Index::checkCurrentScope(const int req) { return consInScope_.find(req) != consInScope_.end(); }

	void Index::changeNumStrikes(const int strikes) { numStrikes_ = strikes; }

	void Index::initializeOptionRequests(EClientL0* EC, const int mktReq) {
		// Send an RTB request for underlying index
		EC->reqRealTimeBars
		(indReq_
			, con_
			, 5
			, *WhatToShow::MIDPOINT
			, UseRTH::OnlyRegularTradingData
		);

		// Now add the exchange and send a mkt data request
		con_.exchange = *Exchange::IB_SMART;
		EC->reqMktData(mktReq, con_, "", true);
	}

	void Index::updateOptionRequests(EClientL0* EC, const double curPrice, IBString todayDate,
		std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> chainData) {

		std::lock_guard<std::mutex> lock(indexMtx);

		std::vector<int> strikes = getStrikes(curPrice, multiple_, numStrikes_);

		// Request queue gets filled by new strikes
		std::queue<Contract> reqQueue;

		// Clear contractsInScope set each time strikes are updated to ensure newly populated strikes always are in scope
		consInScope_.clear();

		for (auto i : strikes) {
			// If the contracts map doesn't already contain the strike, then a new one has come into scope
			if (chainData->find(i) == chainData->end()) {
				// Create new contracts if not in map and add to queue for requests
				Contract con;
				con.symbol = ticker_;
				con.secType = *SecType::OPT;
				con.currency = "USD";
				con.exchange = *Exchange::IB_SMART;
				con.primaryExchange = *Exchange::CBOE;
				con.lastTradeDateOrContractMonth = todayDate;
				con.strike = i;
				con.right = *ContractRight::CALL;

				// Insert into the request queue
				reqQueue.push(con);

				// Now change contract to put and insert
				con.right = *ContractRight::PUT;
				reqQueue.push(con);
			}
			
			// Update the contracts being tracked within the scope
			consInScope_.insert(i);
			consInScope_.insert(i + 1);
		}

		// if (!reqQueue.empty()) OPTIONSCANNER_INFO("{} new contracts added to request queue", reqQueue.size());

		// Now empty the queue and create the requests
		while (!reqQueue.empty()) {
			Contract con = reqQueue.front();

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

			// Update request tracker
			currentReqs_.push_back(req);

			reqQueue.pop();
		}

		// OPTIONSCANNER_DEBUG("New requests sent to queue, total option active requests: {}", currentReqs_.size());
	}

	std::vector<int> getStrikes(const double price, const int multiple, const int numStrikes) {
		std::vector<int> strikes;

		// Round the price down to nearest increment
		int roundedPrice = int(price + (multiple / 2));
		roundedPrice -= roundedPrice % multiple;
		int strikePrice = roundedPrice - (multiple * 5);

		// This will give us 2 times the number of strikes provided
		while (strikePrice <= roundedPrice + (multiple * numStrikes)) {
			strikes.push_back(strikePrice);
			strikePrice += multiple;
		}

		for (auto i : strikes) std::cout << i << " ";
		std::cout << std::endl;

		return strikes;
	}
}