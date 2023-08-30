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

#include "Enums.h"
#include "tWrapper.h"
#include "ContractData.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

//===========================================================
// Current Alert Codes
// 
// Alert codes will be inserted into a vector, each code  
// representing a different aspect of the alert
// 
// This will allow us to create a tree of all alert codes
// and levels, and attach win probabilities
//===========================================================

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


	// Helper function to get bindary code for comparing price to high and low values
	RelativeToMoney distFromPrice(OptionType optType, int strike, double spxPrice);
	TimeOfDay getCurrentHourSlot();
	std::pair<DailyHighsAndLows, LocalHighsAndLows> getHighsAndLows(vector<bool> comparisons);
	int getComparisonCode(vector<bool>& optionComparisons, vector<bool>& SPXComparisons);
	string decodeComparisonCode(int comparisonCode);
	// Get the code based on the current market hour

	struct Alert {
		int reqId;
		double currentPrice;
		TimeFrame tf;
		std::chrono::steady_clock::time_point initTime;

		Alert(int reqId, double currentPrice, TimeFrame tf);
	};

	//=========================================================================================================
	//	AlertData is constructed upon every callback received in OptionScanner from ContractData. It will handle 
	//	all of the data from each instance of an alert, and create nearly all the alert codes upon it's creation.
	//	For each alert created, the program will keep them in a 30 minute queue (subject to change based on success)
	//	and then request data to determine the level of success. This level of success will then be sent to the 
	//	alert code tree to determine the overall success rate.
	//=========================================================================================================
	class AlertData {
	public:
		AlertData(TimeFrame tf, std::shared_ptr<ContractData> cd, std::shared_ptr<ContractData> SPX, std::shared_ptr<Candle> candle);

		// Need a copy constructor for additional alerts like repeated hits

		// vector<int> getAlertCodes() { return alertCodes; }

		// Initial variables to be sent from callback
	public:

		// Incoming Variables
		TimeFrame tf_;
		std::shared_ptr<ContractData> cd_;
		std::shared_ptr<Candle> candle_;

		int code;
		StandardDeviation sdVol;
		StandardDeviation sdPriceDelta;
		StandardDeviation uSdVol; // Underlying
		StandardDeviation uSdPriceDelta; // Underlying
		Candle uBars; // Underlying
		int compCode;

		// Added variables
		int reqId;
		time_t time;
		string dateTime;
		int strike;
		string optionType;
		double vol;
		double priceDelta;
		double closePrice;
		double underlyingPrice;
		double underlyingPriceDelta;
		vector<int> alertCodes;

		// Variables that will be filled to check alert success
	public:
		// This will be provided based on levels
		// -30% will be a failure or 0, 1 will be 15-30%, and so on
		void getSuccessLevel(vector<Candle> postAlertData);
		int successLevel = 0;
	};

	class AlertHandler {
	public:

		void inputAlert(TimeFrame tf, std::shared_ptr<ContractData> cd,
			std::shared_ptr<ContractData> SPX, std::shared_ptr<Candle> candle);

		// **** Be sure to take into account alerts right before close
		void checkQueueUpdates(Candle c, std::unordered_map<int, ContractData*>& contractMap);

		void outputAlert(AlertData* a);

	private:
		// std::unordered_map<int, AlertNode> alertStorage;
		std::queue<AlertData*> alertUpdateQueue;
	};

}