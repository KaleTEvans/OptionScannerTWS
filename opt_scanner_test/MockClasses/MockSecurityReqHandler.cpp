#include "MockSecurityReqHandler.h"

namespace Securities {

	SecurityRequestHandler::SecurityRequestHandler(IBString ticker, int req, int multiple, int numStrikes, std::string secType_) :
		ticker_(ticker), req_(req), multiple_(multiple), numStrikes_(numStrikes)
	{
		con_.symbol = ticker;
		con_.secType = secType_;
		con_.currency = "USD";
		con_.primaryExchange = "CBOE";
	}

	int SecurityRequestHandler::numReqs() const { return currentReqs_.size(); }
	std::vector<int> SecurityRequestHandler::currentActiveReqs() const { return currentReqs_; }
	bool SecurityRequestHandler::checkCurrentScope(const int req) { return consInScope_.find(req) != consInScope_.end(); }

	void SecurityRequestHandler::changeNumStrikes(const int strikes) { numStrikes_ = strikes; }

	void SecurityRequestHandler::initializeOptionRequests(MockClient& EC, const int mktReq) {
		// Send an RTB request for underlying index
		EC.reqRealTimeBars
		(req_
			, con_
			, 5
			, ""
			, UseRTH::OnlyRegularTradingData
		);

		// Now add the exchange and send a mkt data request
		//con_.exchange = *Exchange::IB_SMART;
		//EC.reqMktData(mktReq, con_, "", true);
	}

	void SecurityRequestHandler::updateOptionRequests(MockClient& EC, const double curPrice, IBString todayDate,
		std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> chainData) {

		std::lock_guard<std::mutex> lock(secMtx);

		std::vector<int> strikes = getStrikes(curPrice);

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
				con.secType = "OPT";
				con.currency = "USD";
				con.exchange = "SMART";
				con.primaryExchange = "CBOE";
				con.lastTradeDateOrContractMonth = todayDate;
				con.strike = i;
				con.right = "CALL";

				// Insert into the request queue
				reqQueue.push(con);

				// Now change contract to put and insert
				con.right = "PUT";
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
			if (con.right == "CALL") req = static_cast<int>(con.strike);
			else if (con.right == "PUT") req = static_cast<int>(con.strike + 1);

			// Now create the request
			EC.reqRealTimeBars
			(req
				, con
				, 5
				, ""
				, UseRTH::OnlyRegularTradingData
			);

			// Update request tracker
			currentReqs_.push_back(req);

			reqQueue.pop();
		}

		// OPTIONSCANNER_DEBUG("New requests sent to queue, total option active requests: {}", currentReqs_.size());
		strikesUpdated = true;
	}

	std::vector<int> SecurityRequestHandler::getStrikes(const double price) {
		std::vector<int> strikes;

		// Round the price down to nearest increment
		int roundedPrice = int(price + (multiple_ / 2));
		roundedPrice -= roundedPrice % multiple_;
		int strikePrice = roundedPrice - (multiple_ * 5);

		// This will give us 2 times the number of strikes provided
		while (strikePrice <= roundedPrice + (multiple_ * numStrikes_)) {
			strikes.push_back(strikePrice);
			strikePrice += multiple_;
		}

		for (auto i : strikes) std::cout << i << " ";
		std::cout << std::endl;

		return strikes;
	}
}