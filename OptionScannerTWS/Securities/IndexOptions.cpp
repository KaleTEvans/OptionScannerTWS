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

	void Index::updateOptionRequests(EClientL0* EC, const double curPrice) {
		std::lock_guard<std::mutex> lock(indexMtx);

		std::vector<int> strikes = getStrikes(curPrice, multiple_, numStrikes_);

		// Clear contractsInScope set each time strikes are updated to ensure newly populated strikes always are in scope
		consInScope_.clear();
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