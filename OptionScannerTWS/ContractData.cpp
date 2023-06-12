#include "ContractData.h"

// Helper function to create new candles from time increments
Candle createNewBars(int id, int increment, const vector<Candle>& data) {
	// Need to get total volume, high and low, and the open and close prices
	double open = data[data.size() - increment].open;
	double close = data[data.size() - 1].close;
	long time = data[data.size() - 1].time;

	double high = 0;
	double low = INT_MAX;
	long volume = 0;
	
	for (int i = data.size() - 1; i >= data.size() - increment; i--) {
		if (i < 0) break; // Initial values causing a vector out of bounds issue for some reason
		high = max(high, data[i].high);
		low = min(low, data[i].low);

		volume += data[i].volume;
	}

	Candle c1(id, time, open, high, low, close, volume);

	return c1;
}

ContractData::ContractData(TickerId reqId, Candle initData) : contractId(reqId) {
	// Push the first candle only in the 5 sec array
	fiveSecCandles.push_back(initData);

	// Determine whether call or put using reqId
	if (!reqId % 5) {
		optionType = "CALL";
		contractStrike = reqId;
	}
	else {
		optionType = "PUT";
		contractStrike = reqId - 1;
	}
}

// The input data function will be called each time a new candle is received, and will be where we 
// update each time series vector, stdev and mean. This will also be where we send out alerts
// if any anomalies are found
void ContractData::updateData(Candle c) {
	// Always push to the 5 sec array
	fiveSecCandles.push_back(c);
	// Using the length of the 5 sec array, we will determine if any new candles should be added to the other arrays
	// 6 increments for the 30 sec
	if (fiveSecCandles.size() % 6 == 0) {
		Candle c6 = createNewBars(contractId, 6, fiveSecCandles);
		thirtySecCandles.push_back(c6);

		// Now we'll reference the 30 sec array for the 1min, so we only need to use increments of 2
		if (thirtySecCandles.size() > 0 && thirtySecCandles.size() % 2 == 0) {
			//std::cout << thirtySecCandles.size() << " " << oneMinCandles.size() << std::endl;
			Candle c1 = createNewBars(contractId, 2, thirtySecCandles);
			oneMinCandles.push_back(c1);

			// Referencing the 1 min for the 5min array we can use increments of 5
			if (oneMinCandles.size() > 0 && oneMinCandles.size() % 5 == 0) {
				Candle c5 = createNewBars(contractId, 5, oneMinCandles);
				fiveMinCandles.push_back(c5);
			}
		}
	}
}

//==================================
// Accessor Functions
//===================================
vector<Candle> ContractData::getFiveSecData() const { return fiveSecCandles; }
vector<Candle> ContractData::getThirtySecData() const { return thirtySecCandles; }
vector<Candle> ContractData::getOneMinData() const { return oneMinCandles; }
vector<Candle> ContractData::getFiveMinData() const { return fiveMinCandles; }