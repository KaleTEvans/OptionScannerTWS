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
		DailyHighsAndLows underlyngDailyHL;
		LocalHighsAndLows underlyingLocalHL;
		DailyHighsAndLows optionDailyHL;
		LocalHighsAndLows optionLocalHL;

		AlertTags(OptionType optType, TimeFrame timeFrame, RelativeToMoney rtm, TimeOfDay timeOfDay, VolumeStDev volStDev, 
			VolumeThreshold volThreshold, PriceDelta underlyingPriceDelta, PriceDelta optionPriceDelta, 
			DailyHighsAndLows underlyngDailyHL, LocalHighsAndLows underlyingLocalHL, DailyHighsAndLows optionDailyHL, LocalHighsAndLows optionLocalHL);
	};

	struct Alert {
		int reqId;
		double currentPrice;
		TimeFrame tf;
		std::chrono::steady_clock::time_point initTime;

		Alert(int reqId, double currentPrice, TimeFrame tf);
	};

	class AlertStats {
	public:
		void updateAlertStats(bool win, double percentWon);

		double winRate();
		double averageWin();

	private:
		double winRate_{ 0 };
		double averageWin_{ 0 };

		double totalWins_{ 0 };
		double sumPercentWon_{ 0 };

		double total_{ 0 };
	};

	// Helper function to get bindary code for comparing price to high and low values
	RelativeToMoney distFromPrice(OptionType optType, int strike, double spxPrice);
	TimeOfDay getCurrentHourSlot();
	std::pair<DailyHighsAndLows, LocalHighsAndLows> getHighsAndLows(vector<bool> comparisons);

	class AlertHandler {
	public:

		void inputAlert(TimeFrame tf, std::shared_ptr<ContractData> cd,
			std::shared_ptr<ContractData> SPX, std::shared_ptr<Candle> candle);

		// **** Be sure to take into account alerts right before close
		void checkAlertOutcomes();

		bool doneCheckingAlerts_{ false }; // Change to true on market close
		//void outputAlert();

	private:
		// std::unordered_map<int, AlertNode> alertStorage;
		std::queue<std::pair<AlertTags, Alert>> alertUpdateQueue;
	};

}