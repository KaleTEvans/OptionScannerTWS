#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <unordered_set>
#include <queue>
#include <vector>
#include <memory>

#include "../tWrapper.h"
#include "../ContractData.h"

namespace Securities {

	class Index {
	public:
		Index(IBString ticker, const int req, const int multiple, const int numStrikes);

		// Mutators
		int numReqs() const;
		std::vector<int> currentActiveReqs() const;
		bool checkCurrentScope(const int req);

		void changeNumStrikes(const int strikes);

		void initializeOptionRequests(EClientL0* EC, const int mktReq);
		void updateOptionRequests(EClientL0* EC, const double curPrice, IBString todayDate,
			std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> chainData);


	private:
		IBString ticker_;
		Contract con_;
		const int indReq_;
		const int multiple_;
		int numStrikes_;

		std::vector<int> currentReqs_;
		std::unordered_set<int> consInScope_;

		std::mutex indexMtx;
	};

	std::vector<int> getStrikes(const double price, const int multiple, const int numStrikes);
}

