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

#include "Enums.h"
#include "tWrapper.h"
#include "ContractData.h"
#include "Logger.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

namespace Alerts {

	struct AlertTags {
		OptionType optType;
		TimeFrame timeFrame;
		RelativeToMoney rtm;
		TimeOfDay timeOfDay;
		VolumeStDev volStDev;
		VolumeThreshold volThreshold;
		PriceDelta underlyingPriceDelta;
		PriceDelta optionPriceDelta;
		DailyHighsAndLows underlyingDailyHL;
		LocalHighsAndLows underlyingLocalHL;
		DailyHighsAndLows optionDailyHL;
		LocalHighsAndLows optionLocalHL;

		AlertTags(OptionType optType, TimeFrame timeFrame, RelativeToMoney rtm, TimeOfDay timeOfDay, VolumeStDev volStDev, 
			VolumeThreshold volThreshold, PriceDelta underlyingPriceDelta, PriceDelta optionPriceDelta, 
			DailyHighsAndLows underlyingDailyHL, LocalHighsAndLows underlyingLocalHL, DailyHighsAndLows optionDailyHL, LocalHighsAndLows optionLocalHL);
	};

	// Comparison operator to hold alertTags in a map
	bool operator<(const AlertTags& left, const AlertTags& right);

	struct Alert {
		int reqId;
		double currentPrice;
		TimeFrame tf;
		std::chrono::steady_clock::time_point initTime;
		long unixTime;

		Alert(int reqId, double currentPrice, TimeFrame tf);
	};

	class AlertStats {
	public:
		// A win will be considered 60% profit or more (2:1 ratio to stop loss)
		void updateAlertStats(double win, double percentWon);

		double winRate(); // 0 is a loss, 1 is break even or small win, 2 is 60% or more
		double averageWin();

	private:
		double winRate_{ 0 };
		double averageWin_{ 0 };

		double totalWins_{ 0 };
		double sumPercentWon_{ 0 };

		double total_{ 0 };
	};

	class AlertHandler {
	public:

		AlertHandler(std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> contractMap);
		//~AlertHandler();

		void inputAlert(TimeFrame tf, std::shared_ptr<ContractData> cd,
			std::shared_ptr<ContractData> SPX, std::shared_ptr<Candle> candle);

		// **** Be sure to take into account alerts right before close
		void checkAlertOutcomes();
		void doneCheckingAlerts();

		bool doneCheckingAlerts_{ false }; // Change to true on market close
		//void outputAlert();

	private:
		std::mutex alertMtx_;
		//std::thread alertCheckThread_;
		// std::unordered_map<int, AlertNode> alertStorage;
		std::queue<std::pair<AlertTags, Alert>> alertUpdateQueue;
		// This will store all alert data and stats
		std::map<AlertTags, AlertStats> alertData_;

		// Points to the map being updated by the Option Scanner
		std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> contractMap_;
	};

	//========================================================
	// Helper Functions
	//========================================================

	// Get num strikes ITM or OTM
	RelativeToMoney distFromPrice(OptionType optType, int strike, double spxPrice);
	// Return the time of day during market hours 1-7
	TimeOfDay getCurrentHourSlot();
	// Get proximity to min and max price values
	std::pair<DailyHighsAndLows, LocalHighsAndLows> getHighsAndLows(vector<bool> comparisons);
	// Measure the win rate and the percent win of each alert
	std::pair<double, double> checkWinStats(std::vector<std::shared_ptr<Candle>> prevCandles, Alert a);

}