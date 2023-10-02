#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <unordered_set>
#include <queue>
#include <vector>
#include <memory>

//#include "tWrapper.h"
#include "MockWrapper.h"
#include "MockClient.h"
#include "ContractData.h"

namespace Securities {

	class SecurityRequestHandler {
	public:
		SecurityRequestHandler(IBString ticker, int req, int multiple, int numStrikes, std::string secType);

		// Mutators
		int numReqs() const;
		std::vector<int> currentActiveReqs() const;
		bool checkCurrentScope(const int req);

		void changeNumStrikes(const int strikes);
		std::vector<int> getStrikes(const double price);

		void initializeOptionRequests(MockClient& EC, const int mktReq);
		void updateOptionRequests(MockClient& EC, const double curPrice, IBString todayDate,
			std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> chainData);

		bool strikesUpdated{ false };


	private:
		IBString ticker_;
		Contract con_;
		int multiple_;
		int req_;
		int numStrikes_;
		std::string secType_;

		std::vector<int> currentReqs_;
		std::unordered_set<int> consInScope_;

		std::mutex secMtx;
	};
}