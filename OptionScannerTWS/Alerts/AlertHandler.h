#define _CRT_SECURE_NO_WARNINGS

//=======================================================================
// The AlertHandler will exist in the OptionScanner class, or perhaps
// be a separate entity altogether if monitoring several options
// chains simultaneously in the future. This is where we will send
// the unusal activity to, in the forms of separate alert clases.
// the alert handler will contain all of these, log alerts, while
// also monitoring and chaining multiple alerts for the same contract.
// Lastly the AlertHandler will continuously go through the alerts
// and update them after a specified period of time, to determine
// rate and magnitude of success or failure. In time, the data will
// be continously optimized to determine which alerts are most effective
// and most successful.
//=======================================================================

#pragma once

#include <unordered_map>
#include <queue>
#include <sstream>
#include <map>
#include <memory>

#include "PerformanceResults.h"
#include "Enums.h"
#include "ContractData.h"
#include "Logger.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

namespace Alerts {

	class AlertHandler {
	public:

		AlertHandler(std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> contractMap,
			std::shared_ptr<OptionDB::DatabaseManager> dbm);
		~AlertHandler();

		void inputAlert(std::shared_ptr<CandleTags> candle);

		// **** Be sure to take into account alerts right before close
		void checkAlertOutcomes();
		void doneCheckingAlerts();

		bool doneCheckingAlerts_{ false }; // Change to true on market close
		//void outputAlert();

	private:
		std::mutex alertMtx_;
		std::thread alertCheckThread_;

		std::shared_ptr<OptionDB::DatabaseManager> dbm_;

		// std::unordered_map<int, AlertNode> alertStorage;
		std::queue<std::shared_ptr<PerformanceResults>> alertUpdateQueue;

		// Points to the map being updated by the Option Scanner
		std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> contractMap_;
	};

	// Measure the win rate and the percent win of each alert
	void checkWinStats(std::vector<std::shared_ptr<Candle>> prevCandles, std::shared_ptr<PerformanceResults> a);

}