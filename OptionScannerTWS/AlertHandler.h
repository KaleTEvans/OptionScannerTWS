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
// alertCodes[0] - Call or Put
// 11 - Call
// 22 - PUT
// 
// alertCodes[1] - Timeframe
// 1001 - 5 Second Timeframe
// 1002 - 30 Second Timeframe
// 1003 - 1 Minute Timeframe
// 1004 - 5 Mintue Timeframe

// alertCodes[2] - In the Money or Out of the Money
// 1110 - 1 ITM (In the Money)
// 1120 - 2 ITM
// 1130 - Deep ITM
// 1210 - 1 OTM (Out of the Money)
// 1220 - 2 OTM
// 1230 - Deep OTM

// alertCodes[3] - Time of day
// 2001 - 8:30 CST First Hour
// 2002 - 9:30 CST Second Hour
// 2003 - 10:30 CST Third Hour
// 2004 - 11:30 CST Fourth Hour
// 2005 - 12:30 CST Fifth Hour
// 2006 - 13:30 CST Sixth Hour
// 2007 - 14:30 CST Last Thirty Minutes

// alertCodes[4] - These codes will actually create alerts
// 3011 - Volume over 1 standard deviations
// 3012 - Volume over 2 standard deviations
// 3013 - Volume over 3 standard deviations
// 31xx - Volume over 100, combined with last two digits from above
// 32xx - Volume over 250, combined with last two digits from above
// 33xx - Volume over 500, combined with last two digits from above
// 34xx - Volume over 1000, combined with last two digits from above

// alertCodes[5] - Price delta for accuracy in probability
// 4001 - Price delta less than 1 stdev in underlying price
// 4002 - Price delta less than 2 stdev in underlying price
// 4003 - Price delta less than 1 stdev in contract price
// 4004 - Price delta less than 2 stdev in contract price
// 4005 - Price delta less than 1 stdev in contract and underlying price
// 4006 - Price delta less than 2 stdev in contract and undedrlying price
// 4007 - Price delta greater than 2 stdev

// alertCodes[6] - Contract and Underlying daily highs and lows
// These will be a bit different. Since there are 4 bools for both contract and underlying
// highs and lows, these will be coded into binary. A decode function will also be provided for output

// alertCodes[7] - Extra tags to further determine win rate
// 6021 - Repeated hits (more than 2 of same alert back to back)
// 6022 - Repeated hits (more than 3)
// 6023 - Repeated hits (more than 5)
// 
// 6031 - Low local high - local low delta
// 6032 - High local high - local low delta
// 6033 - Low daily high - daily low delta
// 6034 - High daily high - daily low delta
//
// 6041 - Higher relative volume on underlying
// 6042 - Lower relative volume on underlying



namespace Alerts {

	// Helper function to get bindary code for comparing price to high and low values
	int getComparisonCode(vector<bool>& optionComparisons, vector<bool>& SPXComparisons);
	string decodeComparisonCode(int comparisonCode);
	// Get the code based on the current market hour
	int getCurrentHourSlot();

	class AlertData {
	public:
		AlertData(Candle c, int code, StandardDeviation& sdVol, StandardDeviation& sdPriceDelta,
			double dailyHigh, double dailyLow, double cumulativeVol, double underlyingPrice, int compCode
		);

		// Need a copy constructor for additional alerts like repeated hits

		void outputAlert();

		void updateAlert(vector<Candle>& recentData, int timeFrame);

		// Initial variables to be sent from callback
	public:
		time_t time;
		string dateTime;
		int strike;
		string optionType;
		int code;
		double vol;
		StandardDeviation& sdVol;
		StandardDeviation& sdPriceDelta;
		double priceDelta;
		double closePrice;
		double dailyHigh;
		double dailyLow;
		double cumulativeVol;
		double underlyingPrice;
		int compCode;
		vector<int> alertCodes;

		// Variables that will be filled to check alert success
	public:
		// This will be provided based on levels
		// -30% will be a failure or 0, 1 will be 15-30%, and so on
		void getSuccessLevel(vector<Candle> prior30);
		int successLevel = 0;
	};

	// Each alert node will be associated with a strike, and contain all alerts for both puts and calls
	class AlertNode {
	public:
		AlertNode(AlertData* alertData);

		void addAlertData();
		AlertData* createNewAlert();

		//=========================================
		// These vectors will be organized as such:
		// PUTS[0] = Code 1001
		// PUTS[1] = Code 1002
		// PUTS[2] = Code 1003
		// PUTS[3] = Code 1004
		//=========================================
		vector<vector<AlertData*>> PUTS;
		vector<vector<AlertData*>> CALLS;
	};

	class AlertHandler {
	public:

		void inputAlert(AlertData& a);

		// **** Be sure to take into account alerts right before close
		void checkQueueUpdates(Candle c, std::unordered_map<int, ContractData*>& contractMap);

		void outputAlert(AlertData& a) {
			cout << a.dateTime << " " << a.optionType << " " << a.strike << " | " << a.code << " | Current Price: " << a.closePrice
				<< " | Volume: " << a.vol  << endl;
		}

	private:
		std::unordered_map<int, AlertNode> alertStorage;
		std::queue<AlertData*> alerUpdateQueue;
	};

}