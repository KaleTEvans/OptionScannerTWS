#include "AlertHandler.h"

namespace Alerts {

	//===================================================
	// Alert Data 
	//===================================================

	AlertData::AlertData(Candle c, int code, StandardDeviation sdVol, StandardDeviation sdPriceDelta,
		StandardDeviation uSdVol, StandardDeviation uSdPriceDelta, Candle uBars, int compCode) :
		code(code), sdVol(sdVol), sdPriceDelta(sdPriceDelta), uSdVol(uSdVol), uSdPriceDelta(uSdPriceDelta),
		underlyingPrice(underlyingPrice),compCode(compCode) {
		reqId = c.reqId;
		time = c.time;
		vol = c.volume;
		closePrice = c.close;
		priceDelta = c.high - c.low;
		underlyingPrice = uBars.close;
		underlyingPriceDelta = uBars.high - uBars.low;

		struct tm* timeInfo;
		char buffer[80];

		timeInfo = localtime(&time);
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);

		dateTime = buffer;

		// Determine whether call or put using reqId
		if (c.reqId % 5 == 0) {
			optionType = "CALL";
			alertCodes.push_back(11);
			strike = c.reqId;
		}
		else {
			optionType = "PUT";
			alertCodes.push_back(22);
			strike = c.reqId - 1;
		}

		alertCodes.push_back(code);

		//==================================================================
		// Alert code termining how close to the money the option is
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

		//==========================================================
		// Alert code for time of day

		alertCodes.push_back(2000 + getCurrentHourSlot());

		//=====================================================
		// Alert code to be used for the volume
		int volAlertCode = 3011;

		if (sdVol.numStDev(vol) > 2) volAlertCode++; // 3012
		if (sdVol.numStDev(vol) > 3) volAlertCode++; // 3013

		if (vol > 100) volAlertCode += 100;
		if (vol > 250) volAlertCode += 100; // 200
		if (vol > 500) volAlertCode += 100; // 300
		if (vol > 1000) volAlertCode += 100; // 400

		alertCodes.push_back(volAlertCode);

		//================================================================
		// Alert code for the underlying price delta standard deviations
		int priceDeltaCode = 0;
		double numStDevCon = sdPriceDelta.numStDev(priceDelta);
		double numStDevU = uSdPriceDelta.numStDev(underlyingPriceDelta);

		if (numStDevCon >= 2 && numStDevU >= 2) priceDeltaCode = 4007;
		if (numStDevCon < 2 && numStDevU < 2) priceDeltaCode = 4006;
		if (numStDevCon <= 1 && numStDevU <= 1) priceDeltaCode = 4005;
		if (numStDevCon < 2 && numStDevU >= 2) priceDeltaCode = 4004;
		if (numStDevCon < 1 && numStDevU >= 1) priceDeltaCode = 4003;
		if (numStDevCon >= 2 && numStDevU < 2) priceDeltaCode = 4002;
		if (numStDevCon >= 1 && numStDevU < 1) priceDeltaCode = 4001;
		//===================================
		// Add logger here if no codes returned

		alertCodes.push_back(priceDeltaCode);
		alertCodes.push_back(compCode);
	}

	//========================================================
	// Function to get the AlertData level of success
	//========================================================

	void AlertData::getSuccessLevel(vector<Candle> postAlertData) {
		double maxPrice = INT_MAX;
		double minPrice = 0;
		double percentChange = 0;

		for (size_t i = 0; i < postAlertData.size(); i++) {
			// break the loop if the price drops below 30% before any gains
			if (minPrice > 0 && abs(((postAlertData[i].close - minPrice) / minPrice) * 100) >= 30) {
				// Add a log here to say max loss was met
				break;
			}

			if (maxPrice < INT_MAX) percentChange = abs(((postAlertData[i].close - maxPrice) / maxPrice) * 100);
			if (percentChange >= 15 && percentChange < 30) successLevel = 1;
			if (percentChange > 30 && percentChange < 100) successLevel = 2;
			if (percentChange >= 100) successLevel = 3;

			minPrice = min(minPrice, postAlertData[i].low);
			maxPrice = max(maxPrice, postAlertData[i].high);
		}
	}

	void AlertHandler::inputAlert(AlertData* a) {
		alertUpdateQueue.push(a);

		outputAlert(a);
	}

	void AlertHandler::outputAlert(AlertData* a) {
		std::stringstream ss;
		for (size_t i = 0; i < a->alertCodes.size(); ++i) {
			ss << a->alertCodes[i];
			if (i != a->alertCodes.size() - 1) ss << ' ';
		}

		OPTIONSCANNER_INFO("Alert for {} {} | Current Price: {} | Codes: {} | Volume: {}", 
			a->strike, a->optionType, a->closePrice, ss.str(), a->vol);
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