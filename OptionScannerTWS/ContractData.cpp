#include "ContractData.h"
//#include "Logger.h"

// Helper function to create new candles from time increments
std::shared_ptr<Candle> createNewBars(int id, int increment, const vector<std::shared_ptr<Candle>> data) {
	// Need to get total volume, high and low, and the open and close prices
	double open = data[data.size() - increment]->open();
	double close = data[data.size() - 1]->close();
	long time = data[data.size() - 1]->time();

	double high = 0;
	double low = INT_MAX;
	long volume = 0;
	
	for (size_t i = data.size() - increment; i <= data.size() - 1; i++) {
		high = max(high, data[i]->high());
		low = min(low, data[i]->low());
		volume += data[i]->volume();
	}

	std::shared_ptr<Candle> candle = std::make_shared<Candle>(id, time, open, high, low, close, volume);

	return candle;
}

ContractData::ContractData(TickerId reqId) :
	contractId_{ reqId }, dailyHigh_{ 0 }, dailyLow_{ 10000 }, localHigh_{ 0 }, localLow_{ 10000 }, tempHigh_{ 0 }, tempLow_{ 10000 }
{
	if (reqId % 5 == 0) {
		optType_ = Alerts::OptionType::Call;
		strikePrice_ = reqId;
	}
	else {
		optType_ = Alerts::OptionType::Put;
		strikePrice_ = reqId;
	}

	VPT_.addReqId(reqId);
}

ContractData::ContractData(TickerId reqId, std::shared_ptr<OptionDB::DatabaseManager> dbm) : dbm_(dbm),
contractId_{ reqId }, dailyHigh_{ 0 }, dailyLow_{ 10000 }, localHigh_{ 0 }, localLow_{ 10000 }, tempHigh_{ 0 }, tempLow_{ 10000 }
{
	isUnderlying_ = true;
	setupDatabaseManager(dbm_);
}

// Initiate the SQL connection variable to add db insertion after each candle created
void ContractData::setupDatabaseManager(std::shared_ptr<OptionDB::DatabaseManager> dbm) {
	dbm_ = dbm;
	dbConnect = true;
}

// The input data function will be called each time a new candle is received, and will be where we 
// update each time series vector, stdev and mean. The chaining of if statements ensures that
// each vector has enough values to fill the next timeframe
void ContractData::updateData(std::unique_ptr<Candle> c) {
	//============================================================
	// 5 Second Candle Options
	// ===========================================================
	// Switch to shared pointer
	std::shared_ptr<Candle> fiveSec{ std::move(c) };
	updateContainers(fiveSec, TimeFrame::FiveSecs);

	// Update time of day tag
	updateTimeOfDay(fiveSec->time());

	// Post to db if underlying security
	if (dbConnect && isUnderlying_) {
		dbm_->addToInsertionQueue(fiveSec, TimeFrame::FiveSecs);
	}

	// Update daily high and low values to check relative price
	dailyHigh_ = max(dailyHigh_, fiveSec->high());
	dailyLow_ = min(dailyLow_, fiveSec->low());

	// Wait for the first 30 minutes before updating comparisons
	if (fiveSecCandles_.size() >= 360) updateComparisons();

	std::shared_ptr<CandleTags> fiveSecTags = std::make_shared<CandleTags>(fiveSec, TimeFrame::FiveSecs, optType_, tod_,
		VPT_.volStDev5Sec, VPT_.volThresh5Sec, VPT_.priceDelta5Sec, DHL_, LHL_);

	///////////////////////// 5 Second Alert Options ///////////////////////////////
	// Only try and capture 5 second candles with large volume to avoid adding too much noise to the db
	if (!isUnderlying_ && fiveSecTags->candle.volume() > 100) {
		if (alert_) alert_(fiveSecTags);
	}

	//=============================================================
	// 30 Second Candle Options
	//=============================================================

	if (fiveSecCandles_.size() % 6 == 0 && fiveSecCandles_.size() > 0) {
		std::shared_ptr<Candle> thirtySec{ createNewBars(contractId_, 6, fiveSecCandles_) };
		updateContainers(thirtySec, TimeFrame::ThirtySecs);

		// Post to db
		if (dbConnect && isUnderlying_) dbm_->addToInsertionQueue(thirtySec, TimeFrame::ThirtySecs);

		///////////////////////// 30 Second Alert Options ///////////////////////////////
		std::shared_ptr<CandleTags> thirtySecTags = std::make_shared<CandleTags>(thirtySec, TimeFrame::ThirtySecs, optType_, tod_,
			VPT_.volStDev30Sec, VPT_.volThresh30Sec, VPT_.priceDelta30Sec, DHL_, LHL_);

		if (!isUnderlying_) {
			if (alert_) alert_(thirtySecTags);
		}

		//=================================================================
		// 1 Minute Candle Options
		//=================================================================

		if (thirtySecCandles_.size() > 0 && thirtySecCandles_.size() % 2 == 0) {
			std::shared_ptr<Candle> oneMin{ createNewBars(contractId_, 2, thirtySecCandles_) };
			updateCumulativeVolume(oneMin);
			updateContainers(oneMin, TimeFrame::OneMin);

			// Post to db
			if (dbConnect && isUnderlying_) dbm_->addToInsertionQueue(oneMin, TimeFrame::OneMin);

			std::shared_ptr<CandleTags> oneMinTags = std::make_shared<CandleTags>(oneMin, TimeFrame::OneMin, optType_, tod_,
				VPT_.volStDev1Min, VPT_.volThresh1Min, VPT_.priceDelta1Min, DHL_, LHL_);

			///////////////////////// 1 minute Alert Options ///////////////////////////////
			if (!isUnderlying_) {
				if (alert_) alert_(oneMinTags);
			}

			//====================================================================
			// 5 Minute Candle Options
			//====================================================================

			if (oneMinCandles_.size() > 0 && oneMinCandles_.size() % 5 == 0) {
				std::shared_ptr<Candle> fiveMin{ createNewBars(contractId_, 5, oneMinCandles_) };
				// Every 30 minutes, update the local high and low.
				updateLocalMinMax(fiveMin);
				updateContainers(fiveMin, TimeFrame::FiveMin);

				// Post to db
				if (dbConnect && isUnderlying_) dbm_->addToInsertionQueue(fiveMin, TimeFrame::FiveMin);

				std::shared_ptr<CandleTags> fiveMinTags = std::make_shared<CandleTags>(fiveMin, TimeFrame::FiveMin, optType_, tod_,
					VPT_.volStDev5Min, VPT_.volThresh5Min, VPT_.priceDelta5Min, DHL_, LHL_);

				///////////////////////// 5 Minute Alert Options ///////////////////////////////
				if (!isUnderlying_) {
					if (alert_) alert_(fiveMinTags);
				}
			}
		}
	}

}

//===============================================
// Access Functions
//===============================================

// Accessors
TickerId ContractData::contractId() const { return contractId_; }
int ContractData::strikePrice() const { return strikePrice_; }
Alerts::OptionType ContractData::optType() const { return optType_; }

// Time series accessors
vector<std::shared_ptr<Candle>> ContractData::fiveSecData() const { return fiveSecCandles_; }
vector<std::shared_ptr<Candle>> ContractData::thirtySecData() const { return thirtySecCandles_; }
vector<std::shared_ptr<Candle>> ContractData::oneMinData() const { return oneMinCandles_; }
vector<std::shared_ptr<Candle>> ContractData::fiveMinData() const { return fiveMinCandles_; }

// Other data acessors
double ContractData::currentPrice() const { return fiveSecCandles_.back()->close(); }
double ContractData::dailyHigh() const { return dailyHigh_; }
double ContractData::dailyLow() const { return dailyLow_; }
double ContractData::localHigh() const { return localHigh_; }
double ContractData::localLow() const { return localLow_; }
long long ContractData::totalVol() const { return cumulativeVolume_.back().second; }

vector<std::pair<long, long long>> ContractData::volOverTime() const { return cumulativeVolume_; }

std::shared_ptr<Candle> ContractData::latestCandle(TimeFrame tf) {
	switch (tf)
	{
	case TimeFrame::FiveSecs:
		return fiveSecCandles_.back();
	case TimeFrame::ThirtySecs:
		return thirtySecCandles_.back();
	case TimeFrame::OneMin:
		return oneMinCandles_.back();
	case TimeFrame::FiveMin:
		return fiveMinCandles_.back();
	default:
		OPTIONSCANNER_ERROR("Failed to return most recent candle");
		return {};
	}
}

std::vector<std::shared_ptr<Candle>> ContractData::candlesLast30Minutes() {
	vector<std::shared_ptr<Candle>> res;

	if (fiveSecCandles_.size() >= 360) {
		for (size_t i = fiveSecCandles_.size() - 360; i < fiveSecCandles_.size(); i++) res.push_back(fiveSecCandles_[i]);
	}
	else {
		res = {};
		OPTIONSCANNER_ERROR("Not enough five sec candles to return");
	}

	return res;
}

StandardDeviation ContractData::priceStDev(TimeFrame tf) {
	switch (tf)
	{
	case TimeFrame::FiveSecs:
		return sdPrice5Sec_;
	case TimeFrame::ThirtySecs:
		return sdPrice30Sec_;
	case TimeFrame::OneMin:
		return sdPrice1Min_;
	case TimeFrame::FiveMin:
		return sdPrice5Min_;
	default:
		OPTIONSCANNER_ERROR("Failed to return Price Standard Deviation Object");
		return {};
	}
}

StandardDeviation ContractData::volStDev(TimeFrame tf) {
	switch (tf)
	{
	case TimeFrame::FiveSecs:
		return sdVol5Sec_;
	case TimeFrame::ThirtySecs:
		return sdVol30Sec_;
	case TimeFrame::OneMin:
		return sdVol1Min_;
	case TimeFrame::FiveMin:
		return sdVol5Min_;
	default:
		OPTIONSCANNER_ERROR("Failed to return Volume Standard Deviation Object");
		return {};
	}
}

Alerts::PriceDelta ContractData::priceDelta(TimeFrame tf) {
	switch (tf)
	{
	case TimeFrame::FiveSecs:
		return VPT_.priceDelta5Sec;
	case TimeFrame::ThirtySecs:
		return VPT_.priceDelta30Sec;
	case TimeFrame::OneMin:
		return VPT_.priceDelta1Min;
	case TimeFrame::FiveMin:
		return VPT_.priceDelta5Min;
	default:
		OPTIONSCANNER_ERROR("Failed to return Price Delta Object");
		return {};
	}
}

Alerts::DailyHighsAndLows ContractData::dailyHLComparison() { return DHL_; }
Alerts::LocalHighsAndLows ContractData::localHLComparison() { return LHL_; }

//==============================================
// Helper Functions
//==============================================

void ContractData::updateContainers(std::shared_ptr<Candle> c, TimeFrame tf) {
	double priceStDev = 0;
	double volStDev = 0;
	long volume = 0;

	switch (tf)
	{
	case TimeFrame::FiveSecs:
		fiveSecCandles_.push_back(c);
		sdPrice5Sec_.addValue(c->high() - c->low());
		sdVol5Sec_.addValue(c->volume());

		// Only update stdev tags after 30 minutes of data
		if (fiveMinCandles_.size() >= 360) {
			priceStDev = sdPrice5Sec_.numStDev(c->high() - c->low());
			volStDev = sdVol5Sec_.numStDev(c->volume());
			volume = c->volume();

			VPT_.priceDelta5Sec = VPT_.updatePriceDelta(priceStDev);
			VPT_.volThresh5Sec = VPT_.updateVolThreshold(volume);
			VPT_.volStDev5Sec = VPT_.updateVolStDev(volStDev);
		}

		break;
	case TimeFrame::ThirtySecs:
		thirtySecCandles_.push_back(c);
		sdPrice30Sec_.addValue(c->high() - c->low());
		sdVol30Sec_.addValue(c->volume());

		if (thirtySecCandles_.size() >= 60) {
			priceStDev = sdPrice30Sec_.numStDev(c->high() - c->low());
			volStDev = sdVol30Sec_.numStDev(c->volume());
			volume = c->volume();

			VPT_.priceDelta30Sec = VPT_.updatePriceDelta(priceStDev);
			VPT_.volThresh30Sec = VPT_.updateVolThreshold(volume);
			VPT_.volStDev30Sec = VPT_.updateVolStDev(volStDev);
		}

		break;
	case TimeFrame::OneMin:
		oneMinCandles_.push_back(c);
		sdPrice1Min_.addValue(c->high() - c->low());
		sdVol1Min_.addValue(c->volume());

		if (oneMinCandles_.size() >= 30) {
			priceStDev = sdPrice1Min_.numStDev(c->high() - c->low());
			volStDev = sdVol1Min_.numStDev(c->volume());
			volume = c->volume();

			VPT_.priceDelta1Min = VPT_.updatePriceDelta(priceStDev);
			VPT_.volThresh1Min = VPT_.updateVolThreshold(volume);
			VPT_.volStDev1Min = VPT_.updateVolStDev(volStDev);
		}

		break;
	case TimeFrame::FiveMin:
		fiveMinCandles_.push_back(c);
		sdPrice5Min_.addValue(c->high() - c->low());
		sdVol5Min_.addValue(c->volume());

		if (fiveMinCandles_.size() > 6) {
			priceStDev = sdPrice5Min_.numStDev(c->high() - c->low());
			volStDev = sdVol5Min_.numStDev(c->volume());
			volume = c->volume();

			VPT_.priceDelta5Min = VPT_.updatePriceDelta(priceStDev);
			VPT_.volThresh5Min = VPT_.updateVolThreshold(volume);
			VPT_.volStDev5Min = VPT_.updateVolStDev(volStDev);
		}

		break;
	}
}

void ContractData::updateCumulativeVolume(std::shared_ptr<Candle> c) {
	long long vol = static_cast<long long>(c->volume());
	long time = c->time();

	if (cumulativeVolume_.empty()) {
		std::pair<long, long long> p = { time, vol };
		cumulativeVolume_.push_back(p);
	}
	else {
		long long totalVol = vol + cumulativeVolume_.back().second;
		std::pair<long, long long> p{ time, totalVol };
		cumulativeVolume_.push_back(p);
	}
}

void ContractData::updateComparisons() {
	// Update underlying information
	double lastPrice = fiveSecCandles_.back()->close();

	// Check values against the underlying price, will use 0.1% difference
	if (isWithinXPercent(lastPrice, dailyHigh_, percentDiff)) DHL_ = Alerts::DailyHighsAndLows::NDH;
	else if (isWithinXPercent(lastPrice, dailyLow_, percentDiff)) DHL_ = Alerts::DailyHighsAndLows::NDL;
	else DHL_ = Alerts::DailyHighsAndLows::Inside;

	if (isWithinXPercent(lastPrice, localHigh_, percentDiff) || lastPrice > localHigh_) LHL_ = Alerts::LocalHighsAndLows::NLH;
	else if (isWithinXPercent(lastPrice, localLow_, percentDiff) || lastPrice < localLow_) LHL_ = Alerts::LocalHighsAndLows::NLL;
	else LHL_ = Alerts::LocalHighsAndLows::Inside;
}

void ContractData::updateLocalMinMax(std::shared_ptr<Candle> c) {
	// Every 30 minutes, update the local high and low. Temp high and low will serve to track these values in between
	tempHigh_ = max(tempHigh_, c->high());
	tempLow_ = min(tempLow_, c->low());

	if (fiveMinCandles_.size() % 6 == 0 && fiveMinCandles_.size() > 0) {
		localHigh_ = tempHigh_;
		localLow_ = tempLow_;
		tempHigh_ = 0;
		tempLow_ = 10000;
	}
}

//===========================================================
// Tag Update Functions
//===========================================================

void ContractData::updateTimeOfDay(long unixTime) {

	std::time_t currentTime;
	currentTime = static_cast<time_t>(unixTime);

	// Convert the current time to local time
	tm* localTime = std::localtime(&currentTime);

	// Get the current hour and minute
	int currentHour = localTime->tm_hour;
	int currentMinute = localTime->tm_min;

	// Calculate the time in minutes since 8:30 AM
	int minutesSince830AM = (currentHour - 8) * 60 + currentMinute - 30;

	// Calculate the slot number (1 to 7)
	int slotNumber = minutesSince830AM / 60 + 1;

	switch (slotNumber)
	{
	case 1:
		tod_ = Alerts::TimeOfDay::Hour1;
		break;
	case 2:
		tod_ = Alerts::TimeOfDay::Hour2;
		break;
	case 3:
		tod_ = Alerts::TimeOfDay::Hour3;
		break;
	case 4:
		tod_ = Alerts::TimeOfDay::Hour4;
		break;
	case 5:
		tod_ = Alerts::TimeOfDay::Hour5;
		break;
	case 6:
		tod_ = Alerts::TimeOfDay::Hour6;
		break;
	case 7:
		tod_ = Alerts::TimeOfDay::Hour7;
		break;
	default:
		OPTIONSCANNER_ERROR("Error occured calculating time of day");

		break;
	}
}

void VolAndPriceTags::addReqId(int req) { reqId = req; }

Alerts::PriceDelta VolAndPriceTags::updatePriceDelta(double priceStDev) {
	if (priceStDev < 1) return Alerts::PriceDelta::Under1;
	if (priceStDev >= 1 && priceStDev <= 2) return Alerts::PriceDelta::Under2;
	if (priceStDev > 2) return Alerts::PriceDelta::Over2;

	OPTIONSCANNER_ERROR("Invalid Price Delta: {} for ReqID: {}", priceStDev, reqId);
}

Alerts::VolumeStDev VolAndPriceTags::updateVolStDev(double volStDev) {
	if (volStDev <= 1) return Alerts::VolumeStDev::LowVol;
	if (volStDev > 1 && volStDev <= 2) return Alerts::VolumeStDev::Over1;
	if (volStDev > 2 && volStDev <= 3) return Alerts::VolumeStDev::Over2;
	if (volStDev > 3 && volStDev <= 4) return Alerts::VolumeStDev::Over3;
	if (volStDev > 4) return Alerts::VolumeStDev::Over4;

	OPTIONSCANNER_ERROR("Invalid Volume Stdev");
}

Alerts::VolumeThreshold VolAndPriceTags::updateVolThreshold(long volume) {
	if (volume < 100) return Alerts::VolumeThreshold::LowVol;
	if (volume >= 100 && volume < 250) return Alerts::VolumeThreshold::Vol100;
	if (volume >= 250 && volume < 500) return Alerts::VolumeThreshold::Vol250;
	if (volume >= 500 && volume < 1000) return Alerts::VolumeThreshold::Vol500;
	if (volume >= 1000) return Alerts::VolumeThreshold::Vol1000;

	OPTIONSCANNER_ERROR("Invalid Volume Value");
}

Alerts::RelativeToMoney distFromPrice(Alerts::OptionType optType, int strike, double spxPrice) {
	Alerts::RelativeToMoney rtm;

	double priceDifference = std::abs(spxPrice - static_cast<double>(strike));

	if (spxPrice == strike) {
		rtm = Alerts::RelativeToMoney::ATM;
	}
	else {
		int strikesOTM = static_cast<int>(std::ceil(priceDifference / 5));

		if (strikesOTM == 1) {
			if (spxPrice > strike) (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::OTM1 : rtm = Alerts::RelativeToMoney::ITM1;
			else (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::ITM1 : rtm = Alerts::RelativeToMoney::OTM1;
		}
		else if (strikesOTM == 2) {
			if (spxPrice > strike) (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::OTM2 : rtm = Alerts::RelativeToMoney::ITM2;
			else (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::ITM2 : rtm = Alerts::RelativeToMoney::OTM2;
		}
		else if (strikesOTM == 3) {
			if (spxPrice > strike) (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::OTM3 : rtm = Alerts::RelativeToMoney::ITM3;
			else (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::ITM3 : rtm = Alerts::RelativeToMoney::OTM3;
		}
		else if (strikesOTM == 4) {
			if (spxPrice > strike) (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::OTM4 : rtm = Alerts::RelativeToMoney::ITM4;
			else (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::ITM4 : rtm = Alerts::RelativeToMoney::OTM4;
		}
		else if (strikesOTM >= 5) {
			if (spxPrice > strike) (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::DeepOTM : rtm = Alerts::RelativeToMoney::DeepITM;
			else (optType == Alerts::OptionType::Call) ? rtm = Alerts::RelativeToMoney::DeepITM : rtm = Alerts::RelativeToMoney::DeepOTM;
		}
		else {
			OPTIONSCANNER_ERROR("Error ocurred calculating distance from price");
		}
	}

	return rtm;
}
