
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

#include "tWrapper.h"

using std::cout;
using std::endl;

//==============================
// Current Alert Codes
//==============================
// 1001 - 5 Second Timeframe
// 1002 - 30 Second Timeframe
// 1003 - 1 Minute Timeframe
// 1004 - 5 Mintue Timeframe

class Alert {
public:
	Alert(Candle c, int code, double stDevVol, double stDevPriceDelta) :
		code(code), stDevVol(stDevVol), stDevPriceDelta(stDevPriceDelta) {
		time = c.time;
		vol = c.volume;
		closePrice = c.close;
		priceDelta = c.high - c.low;

		// Determine whether call or put using reqId
		if (c.reqId % 5 == 0) {
			optionType = "CALL";
			strike = c.reqId;
		}
		else {
			optionType = "PUT";
			strike = c.reqId - 1;
		}

		struct tm* timeInfo;
		char buffer[80];

		timeInfo = localtime(&time);
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);

		dateTime = buffer;
	}

	// Initial variables to be sent from callback
public:
	time_t time;
	string dateTime;
	int strike;
	string optionType;
	int code;
	double closePrice;
	double vol;
	double stDevVol;
	double priceDelta;
	double stDevPriceDelta;
};

class AlertHandler {
public:

	void outputAlert(Alert& a) {
		cout << a.dateTime << " " << a.optionType << " " << a.strike << " | " << a.code << " | Current Price: " << a.closePrice 
			<< " | Volume: " << a.vol << " Volume StDev: " << a.stDevVol << endl;
	}
};
