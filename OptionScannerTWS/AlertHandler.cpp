#include "AlertHandler.h"

namespace Alerts {

	AlertTags::AlertTags(OptionType optType, TimeFrame timeFrame, RelativeToMoney rtm, TimeOfDay timeOfDay, VolumeStDev volStDev, 
		VolumeThreshold volThreshold, PriceDelta underlyingPriceDelta, PriceDelta optionPriceDelta, DailyHighsAndLows underlyngDailyHL, 
		LocalHighsAndLows underlyingLocalHL, DailyHighsAndLows optionDailyHL, LocalHighsAndLows optionLocalHL) :
		optType(optType), timeFrame(timeFrame), rtm(rtm), timeOfDay(timeOfDay), volStDev(volStDev), volThreshold(volThreshold),
		underlyingPriceDelta(underlyingPriceDelta), optionPriceDelta(optionPriceDelta), underlyngDailyHL(underlyngDailyHL), 
		underlyingLocalHL(underlyingLocalHL), optionDailyHL(optionDailyHL), optionLocalHL(optionLocalHL) {}

	Alert::Alert(int reqId, double currentPrice, TimeFrame tf) :
		reqId(reqId), currentPrice(currentPrice), tf(tf) {

		initTime = std::chrono::steady_clock::now();
	}

	//===================================================
	// Alert Data 
	//===================================================

	void AlertHandler::inputAlert(TimeFrame tf, std::shared_ptr<ContractData> cd, 
		std::shared_ptr<ContractData> SPX, std::shared_ptr<Candle> candle) {

		std::unique_ptr<Alert> alert = std::make_unique<Alert>(candle->reqId(), candle->close(), tf);

		int strike = 0;
		OptionType optType;
		RelativeToMoney rtm;
		TimeOfDay tod;
		VolumeStDev volStDev;
		VolumeThreshold volThreshold;

		PriceDelta optionDelta;
		PriceDelta underlyingDelta;

		// Determine whether call or put using reqId
		if (candle->reqId() % 5 == 0) {
			optType = OptionType::Call;
			strike = candle->reqId();
		}
		else {
			optType = OptionType::Put;
			strike = candle->reqId() - 1;
		}

		//==================================================================
		// Alert code determining how close to the money the option is
		
		double spxPrice = SPX->currentPrice();
		rtm = distFromPrice(optType, strike, spxPrice);

		//==========================================================
		// Alert code for time of day

		tod = getCurrentHourSlot();

		//=====================================================
		// Alert code to be used for the volume

		StandardDeviation sdVol = cd->volStDev(tf);
		long vol = candle->volume();

		if (sdVol.numStDev(vol) <= 1) volStDev = VolumeStDev::LowVol;
		else if (sdVol.numStDev(vol) > 1 && sdVol.numStDev(vol) <= 2) volStDev = VolumeStDev::Over1;
		else if (sdVol.numStDev(vol) > 2 && sdVol.numStDev(vol) <= 3) volStDev = VolumeStDev::Over2;
		else if (sdVol.numStDev(vol) > 3 && sdVol.numStDev(vol) <= 4) volStDev = VolumeStDev::Over3;
		else volStDev = VolumeStDev::Over4;

		if (vol <= 100) volThreshold = VolumeThreshold::LowVol;
		else if (vol > 100 && vol <= 250) volThreshold = VolumeThreshold::Vol100;
		else if (vol > 250 && vol <= 500) volThreshold = VolumeThreshold::Vol250;
		else if (vol > 500 && vol <= 1000) volThreshold = VolumeThreshold::Vol500;
		else volThreshold = VolumeThreshold::Vol1000;

		//================================================================
		// Alert code for the underlying price delta standard deviations
		
		StandardDeviation sdPriceOpt = cd->priceStDev(tf);
		StandardDeviation sdPriceSpx = SPX->priceStDev(tf);
		std::shared_ptr<Candle> spxCandle = SPX->latestCandle(tf);

		double optPriceDev = sdPriceOpt.numStDev(candle->high() - candle->low());
		double spxPriceDev = sdPriceSpx.numStDev(spxCandle->high() - spxCandle->low());

		if (optPriceDev < 1) optionDelta = PriceDelta::Under1;
		else if (optPriceDev >= 1 && optPriceDev < 2) optionDelta = PriceDelta::Under2;
		else optionDelta = PriceDelta::Over2;

		if (spxPriceDev < 1) underlyingDelta = PriceDelta::Under1;
		else if (spxPriceDev >= 1 && spxPriceDev < 2) underlyingDelta = PriceDelta::Under2;
		else underlyingDelta = PriceDelta::Over2;

		//==================================================
		// Alert code for distance from highs and lows
		
		std::pair<DailyHighsAndLows, LocalHighsAndLows> optHighLowVals = getHighsAndLows(cd->highLowComparisons());
		std::pair<DailyHighsAndLows, LocalHighsAndLows> spxHighLowVals = getHighsAndLows(SPX->highLowComparisons());

		AlertTags alertTags(optType, tf, rtm, tod, volStDev, volThreshold, underlyingDelta, optionDelta, spxHighLowVals.first,
			spxHighLowVals.second, optHighLowVals.first, optHighLowVals.second);
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

	RelativeToMoney distFromPrice(OptionType optType, int strike, double spxPrice) {
		RelativeToMoney rtm;

		double priceDifference = std::abs(spxPrice - static_cast<double>(strike));

		if (spxPrice == strike) {
			rtm = RelativeToMoney::ATM;
		}
		else {
			int strikesOTM = static_cast<int>(std::ceil(priceDifference / 5));

			switch (strikesOTM)
			{
			case 1:
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::OTM1 : rtm = RelativeToMoney::ITM1;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::ITM1 : rtm = RelativeToMoney::OTM1;
				break;
			case 2:
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::OTM2 : rtm = RelativeToMoney::ITM2;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::ITM2 : rtm = RelativeToMoney::OTM2;
				break;
			case 3:
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::OTM3 : rtm = RelativeToMoney::ITM3;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::ITM3 : rtm = RelativeToMoney::OTM3;
				break;
			case 4:
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::OTM4 : rtm = RelativeToMoney::ITM4;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::ITM4 : rtm = RelativeToMoney::OTM4;
				break;
			case 5:
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::OTM5 : rtm = RelativeToMoney::ITM5;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::ITM5 : rtm = RelativeToMoney::OTM5;
				break;
			default:
				std::cout << "Error ocurred calculating distance from price" << std::endl;
				break;
			}
		}

		return rtm;
	}

	TimeOfDay getCurrentHourSlot() {
		TimeOfDay tod;

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

		switch (slotNumber)
		{
		case 1:
			tod = TimeOfDay::Hour1;
			break;
		case 2:
			tod = TimeOfDay::Hour2;
			break;
		case 3:
			tod = TimeOfDay::Hour3;
			break;
		case 4:
			tod = TimeOfDay::Hour4;
			break;
		case 5:
			tod = TimeOfDay::Hour5;
			break;
		case 6:
			tod = TimeOfDay::Hour6;
			break;
		case 7:
			tod = TimeOfDay::Hour7;
			break;
		default:
			std::cout << "Error occured calculating time of day" << std::endl;
			break;
		}

		return tod;
	}

	std::pair<DailyHighsAndLows, LocalHighsAndLows> getHighsAndLows(vector<bool> comparisons) {
		DailyHighsAndLows DHL;
		LocalHighsAndLows LHL;

		if (comparisons[0]) DHL = DailyHighsAndLows::NDL;
		else if (comparisons[1]) DHL = DailyHighsAndLows::NDH;

		if (comparisons[2]) LHL = LocalHighsAndLows::NLL;
		else if (comparisons[3]) LHL = LocalHighsAndLows::NLH;

		return { DHL, LHL };
	}

}