#include "AlertHandler.h"

namespace Alerts {

	//===================================================
	// Alert Data Constructor
	//===================================================

	AlertData::AlertData(Candle c, int code, double stDevVol, double stDevPriceDelta,
		double dailyHigh, double dailyLow, double cumulativeVol, double underlyingPrice, int compCode) :
		code(code), stDevVol(stDevVol), stDevPriceDelta(stDevPriceDelta),
		dailyHigh(dailyHigh), dailyLow(dailyLow), cumulativeVol(cumulativeVol), underlyingPrice(underlyingPrice),
		compCode(compCode) {
		time = c.time;
		vol = c.volume;
		closePrice = c.close;
		priceDelta = c.high - c.low;

		// Determine whether call or put using reqId
		if (c.reqId % 5 == 0) {
			optionType = "CALL";
			alertCodes.push_back(01);
			strike = c.reqId;
		}
		else {
			optionType = "PUT";
			alertCodes.push_back(02);
			strike = c.reqId - 1;
		}

		alertCodes.push_back(code);

		struct tm* timeInfo;
		char buffer[80];

		timeInfo = localtime(&time);
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);

		dateTime = buffer;

		// Determine how close to the money the option is
		double difference = strike - underlyingPrice;
		if (difference <= 5 && difference > 0) (optionType == "CALL") ? alertCodes.push_back(1210) : alertCodes.push_back(1110);
		else if (difference <= 10 && difference > 5) (optionType == "CALL") ? alertCodes.push_back(1220) : alertCodes.push_back(1120);
		else if (difference > 10) (optionType == "CALL") ? alertCodes.push_back(1230) : alertCodes.push_back(1130);
		else if (difference <= 0 && difference > -5) (optionType == "CALL") ? alertCodes.push_back(1110) : alertCodes.push_back(1210);
		else if (difference <= -5 && difference > -10) (optionType == "CALL") ? alertCodes.push_back(1120) : alertCodes.push_back(1220);
		else if (difference < -10) (optionType == "CALL") ? alertCodes.push_back(1130) : alertCodes.push_back(1230);
		else {
			cout << "Error, could not retrieve OTM or ITM code for contract: " << strike << " difference: " << difference << endl;
		}

		// Push back the alert code for time of day
		alertCodes.push_back(2000 + getCurrentHourSlot());

		// Now determine the alert code to be used for the volume
		int volAlertCode;

		if (vol > 20 * stDevVol) volAlertCode = 3014;
		else if (vol > 15 * stDevVol) volAlertCode = 3013;
		else if (vol > 10 * stDevVol) volAlertCode = 3012;
		else if (vol > 5 * stDevVol) volAlertCode = 3011;
		else volAlertCode = 3010;

		if (vol > 1000) volAlertCode += 400;
		else if (vol > 500) volAlertCode += 300;
		else if (vol > 250) volAlertCode += 200;
		else if (vol > 100) volAlertCode += 100;

		alertCodes.push_back(volAlertCode);
		alertCodes.push_back(compCode);
	}

	//========================================================
	// Helper Functions
	//========================================================

	int getCurrentHourSlot() {
		// Get the current time
		auto now = std::chrono::system_clock::now();
		time_t currentTime = std::chrono::system_clock::to_time_t(now);

		// Convert the current time to local time
		tm* localTime = std::localtime(&currentTime);

		// Get the current hour and minute
		int currentHour = localTime->tm_hour;
		int currentMinute = localTime->tm_min;

		// Calculate the time in minutes since 8:30 AM
		int minutesSince830AM = (currentHour - 8) * 60 + currentMinute - 30;

		// Calculate the slot number (1 to 7)
		int slotNumber = minutesSince830AM / 60 + 1;

		return slotNumber;
	}

	int getComparisonCode(vector<bool>& optionComparisons, vector<bool>& SPXComparisons) {
		// Now determine the alert code for the comparison of highs and lows
		int comparisonCode = 0;

		// Convert v1 to an integer value (bits 0-3)
		for (int i = 0; i < 4; ++i) {
			if (optionComparisons[i]) {
				comparisonCode |= (1 << (optionComparisons.size() - 1 - i));
			}
		}

		// Convert v2 to an integer value (bits 4-7)
		for (int i = 0; i < 4; ++i) {
			if (SPXComparisons[i]) {
				comparisonCode |= (1 << (2 * SPXComparisons.size() - 1 - i));
			}
		}

		return comparisonCode;
	}

	string decodeComparisonCode(int combinationValue) {
		string output;
		int size = 4;

		// Check each bit for v1
		output += "| Con | ";
		for (int i = 0; i < size; ++i) {
			if (combinationValue & (1 << (size - 1 - i))) {
				//output += " | ";
				switch (i) {
				case 0:
					output += "NDL";
					break;
				case 1:
					output += "NDH";
					break;
				case 2:
					output += "NLL";
					break;
				case 3:
					output += "NLH";
					break;
				}
				output += " ";
			}
		}

		// Check each bit for v2
		output += "| SPX | ";
		for (int i = 0; i < size; ++i) {
			if (combinationValue & (1 << (2 * size - 1 - i))) {
				//output += "SPX | ";
				switch (i) {
				case 0:
					output += "NDL";
					break;
				case 1:
					output += "NDH";
					break;
				case 2:
					output += "NLL";
					break;
				case 3:
					output += "NLH";
					break;
				}
				output += " ";
			}
		}

		// Remove the trailing space if any
		if (!output.empty()) {
			output.pop_back();
		}

		return output;
	}
}