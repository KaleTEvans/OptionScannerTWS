#pragma once

#include <iostream>
#include <unordered_set>
#include <queue>
#include <vector>
#include <memory>

#include "../tWrapper.h"

namespace Securities {

	class Index {
	public:
		Index(IBString ticker, const int req, const int multiple, const int numStrikes);

		void initializeOptionRequests(EClientL0* EC, const int mktReq);
		void updateOptionRequests(EClientL0* EC, const double curPrice);


	private:
		IBString ticker_;
		Contract con_;
		const int indReq_;
		const int multiple_;
		const int numStrikes_;

		std::vector<int> currentReqs_;
		std::unordered_set<int> consInScope_;

		std::mutex indexMtx;
	};

	std::vector<int> getStrikes(const double price, const int multiple, const int numStrikes);
}

namespace Securities {

	//namespace Index {

		Contract createIndexContract(IBString ticker) {
			Contract con;
			con.symbol = ticker;
			con.secType = *SecType::IND;
			con.currency = "USD";
			con.primaryExchange = *Exchange::CBOE;

			return con;
		}

		void initializeOptionRequests(EClientL0* EC, Contract con, const int req, const int mktReq) {
			// begin by sending an RTB request for the index
			EC->reqRealTimeBars
			(req
				, con
				, 5
				, *WhatToShow::MIDPOINT
				, UseRTH::OnlyRegularTradingData
			);

			// Now add the exchange and send a mkt data request
			con.exchange = *Exchange::IB_SMART;
			EC->reqMktData(mktReq, con, "", true);
		}

		//std::unordered_set<int> updateOptionRequests(EClientL0* EC, )
	//}
}