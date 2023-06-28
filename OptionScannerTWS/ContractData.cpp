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
	
	for (size_t i = data.size() - 1; i > 0 && i > data.size() - increment; i--) {
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
	// Update the standard deviation class for the first 5 sec candle
	sd5Sec.addValue(initData.high - initData.low);
	sdVol5Sec.addValue(initData.volume);

	// Determine whether call or put using reqId
	if (reqId % 5 == 0) {
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

	// Update stdev values for 5 sec array
	sd5Sec.addValue(c.high - c.low);
	sdVol5Sec.addValue(c.volume);

	if (c.volume > 2 * (sdVol5Sec.getStDev())) {
		if (alert_) alert_(101, sdVol5Sec.getStDev(), c);
	}

	// Using the length of the 5 sec array, we will determine if any new candles should be added to the other arrays
	// 6 increments for the 30 sec
	if (fiveSecCandles.size() % 6 == 0) {
		Candle c6 = createNewBars(contractId, 6, fiveSecCandles);
		thirtySecCandles.push_back(c6);

		// Update stdev values for 30 sec array
		sd30Sec.addValue(c6.high - c6.low);
		sdVol30Sec.addValue(c6.volume);

		// Now we'll reference the 30 sec array for the 1min, so we only need to use increments of 2
		if (thirtySecCandles.size() > 0 && thirtySecCandles.size() % 2 == 0) {
			//std::cout << thirtySecCandles.size() << " " << oneMinCandles.size() << std::endl;
			Candle c1 = createNewBars(contractId, 2, thirtySecCandles);
			oneMinCandles.push_back(c1);

			// Update stdev values for 1 min array
			sd1Min.addValue(c1.high - c1.low);
			sdVol1Min.addValue(c1.volume);

			// Referencing the 1 min for the 5min array we can use increments of 5
			if (oneMinCandles.size() > 0 && oneMinCandles.size() % 5 == 0) {
				Candle c5 = createNewBars(contractId, 5, oneMinCandles);
				fiveMinCandles.push_back(c5);

				// Update stdev values for 5 minute array
				sd5Min.addValue(c5.high - c5.low);
				sdVol5Min.addValue(c5.volume);

				// Update cumulative volume for historical records
				std::pair<long, long> p = { c5.time, c5.volume };
				cumulativeVolume.push_back(p);

				// Update daily high and low values to check relative price
				dailyHigh = max(dailyHigh, c5.high);
				dailyLow = min(dailyLow, c5.low);
			}
		}
	}

}

void ContractData::registerAlert(AlertFunction alert) {
	alert_ = std::move(alert);
}

//==================================
// Accessor Functions
//===================================
vector<Candle> ContractData::getFiveSecData() const { return fiveSecCandles; }
vector<Candle> ContractData::getThirtySecData() const { return thirtySecCandles; }
vector<Candle> ContractData::getOneMinData() const { return oneMinCandles; }
vector<Candle> ContractData::getFiveMinData() const { return fiveMinCandles; }