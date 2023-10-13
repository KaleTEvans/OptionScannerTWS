#include "AlertHandler.h"

namespace Alerts {

	Alert::Alert(int reqId, int strike, double currentPrice, long unixTIme, TimeFrame tf) :
		reqId(reqId), strike(strike), currentPrice(currentPrice), unixTime(unixTIme), tf(tf) {

		initTime = std::chrono::steady_clock::now();

		time_t tempTime = std::chrono::duration_cast<std::chrono::seconds>(initTime.time_since_epoch()).count();
		//unixTime = static_cast<long>(tempTime);
	}

	//===================================================
	// Alert Handler
	//===================================================

	AlertHandler::AlertHandler(std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> contractMap) :
		contractMap_(contractMap) {
	
		// Initialize the alert tag stats class for win rate handling
		alertTagStats = std::make_unique<AlertTagStats>();
		// Start a thread to check the alerts
		alertCheckThread_ = std::thread(&AlertHandler::checkAlertOutcomes, this);
	}

	void AlertHandler::inputAlert(TimeFrame tf, std::shared_ptr<ContractData> cd, 
		std::shared_ptr<ContractData> SPX, std::shared_ptr<Candle> candle) {

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

		Alert alert(candle->reqId(), strike, candle->close(), candle->time(), tf);

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

		if (vol > 50) {
			OPTIONSCANNER_INFO("{} at Strike: {} | Current Volume: {} [ TAGS: TimeFrame: {} | {} | {} | Standard Deviations: {} | Total Vol: {} ]",
				EnumString::option_type(optType), strike, vol, time_frame(tf), EnumString::relative_to_money(rtm),
				EnumString::time_of_day(tod), EnumString::vol_st_dev(volStDev), EnumString::vol_threshold(volThreshold));

			AlertStats stats = alertTagStats->alertSpecificStats(alertTags);
			if (stats.totalAlerts() == 0) {
				OPTIONSCANNER_INFO("Currently no data available for alert: {} {}", EnumString::option_type(optType), strike);
			}
			else {
				OPTIONSCANNER_INFO("Data for alert: {} {} | Win Rate: {} | Average Win: {}",
					EnumString::option_type(optType), strike, stats.winRate(), stats.averageWin());
			}
		}
		//std::cout << "Alert added for " << optType << " at Strike: " << strike << " | Current Volume: " << vol << std::endl;

		alertUpdateQueue.push({ alertTags, alert });
	}

	void AlertHandler::checkAlertOutcomes() {
		// Start time to reference and log win rate data every 30 minutes
		std::chrono::steady_clock::time_point refTime = std::chrono::steady_clock::now();

		while (!doneCheckingAlerts_) {
			while (!alertUpdateQueue.empty()) {
				// Check current time to see if 30 minutes have passed since the first alert was added to the queue
				std::chrono::steady_clock::time_point prevAlertTIme = alertUpdateQueue.front().second.initTime;
				std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
				std::chrono::minutes elsapsedTime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - prevAlertTIme);

				if (elsapsedTime >= std::chrono::minutes(30)) {
					std::unique_lock<std::mutex> lock(alertMtx_);

					AlertTags tags = alertUpdateQueue.front().first;
					Alert a = alertUpdateQueue.front().second;
					// Access data from the contract map
					std::vector<std::shared_ptr<Candle>> prevCandles = contractMap_->at(a.reqId)->candlesLast30Minutes();

					// just use the lock to access the contract map
					lock.unlock();

					// Retrieve pair of win bool and percent win
					std::pair<double, double> winStats = checkWinStats(prevCandles, a);

					/*OPTIONSCANNER_DEBUG("Update complete for {} at strike: {} | [ TimeFrame: {} | {} | Total Vol: {} ] [ Win Pct: {} ]",
						EnumString::option_type(tags.optType), a.strike, EnumString::time_frame(tags.timeFrame), EnumString::realtive_to_money(tags.rtm),
						EnumString::vol_threshold(tags.volThreshold), winStats.second);*/

					// Now update the alert win rate if in the map
					alertTagStats->updateStats(tags, winStats.first, winStats.second);

					alertUpdateQueue.pop();
				}

				// Log alert tag values every 30 mintues
				std::chrono::minutes elapsedLogTime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - refTime);
				if (elapsedLogTime >= std::chrono::minutes(30)) {
					alertTagStats->logAllTagStats();
					refTime = std::chrono::steady_clock::now();
				}

				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}
	}

	void AlertHandler::doneCheckingAlerts() {
		doneCheckingAlerts_ = true;
		alertCheckThread_.join();
	}

	//========================================================
	// Helper Functions
	//========================================================

	std::pair<double, double> checkWinStats(std::vector<std::shared_ptr<Candle>> prevCandles, Alert a) {
		// Ensure we start at a vector candle that is after the alert time
		// If we are close to the market close, it won't be a full 30 minutes
		size_t i = 0;
		while (a.unixTime > prevCandles[i]->time()) i++;

		double startPrice = a.currentPrice;
		double maxPrice = startPrice;
		double minPrice = startPrice;
		double percentChangeHigh = 0;
		double percentChangeLow = 0;

		for (i; i < prevCandles.size(); i++) {

			// If percentChangeLow gets to -30% break and compare to percentChangeHigh
			if (percentChangeLow <= -30.0) {
				break;
			}

			minPrice = min(minPrice, prevCandles[i]->low());
			maxPrice = max(maxPrice, prevCandles[i]->high());

			percentChangeLow = ((minPrice - startPrice) / startPrice) * 100;
			percentChangeHigh = ((maxPrice - startPrice) / startPrice) * 100;
		}

		if (percentChangeHigh <= 0) {
			return { 0, 0 };
		}
		else  if (percentChangeHigh > 0 && percentChangeHigh < 60.0) {
			return { 0.5, percentChangeHigh };
		}
		else {
			return { 1, percentChangeHigh };
		}
	}

	RelativeToMoney distFromPrice(OptionType optType, int strike, double spxPrice) {
		RelativeToMoney rtm;

		double priceDifference = std::abs(spxPrice - static_cast<double>(strike));

		if (spxPrice == strike) {
			rtm = RelativeToMoney::ATM;
		}
		else {
			int strikesOTM = static_cast<int>(std::ceil(priceDifference / 5));

			if (strikesOTM == 1) {
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::OTM1 : rtm = RelativeToMoney::ITM1;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::ITM1 : rtm = RelativeToMoney::OTM1;
			}
			else if (strikesOTM == 2) {
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::OTM2 : rtm = RelativeToMoney::ITM2;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::ITM2 : rtm = RelativeToMoney::OTM2;
			}
			else if (strikesOTM == 3) {
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::OTM3 : rtm = RelativeToMoney::ITM3;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::ITM3 : rtm = RelativeToMoney::OTM3;
			} 
			else if (strikesOTM == 4) {
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::OTM4 : rtm = RelativeToMoney::ITM4;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::ITM4 : rtm = RelativeToMoney::OTM4;
			} 
			else if (strikesOTM >= 5) {
				if (spxPrice > strike) (optType == OptionType::Call) ? rtm = RelativeToMoney::DeepOTM : rtm = RelativeToMoney::DeepITM;
				else (optType == OptionType::Call) ? rtm = RelativeToMoney::DeepITM : rtm = RelativeToMoney::DeepOTM;
			}
			else {
				std::cout << "Error ocurred calculating distance from price" << std::endl;
			}
		}

		return rtm;
	}

	TimeOfDay getCurrentHourSlot(long unixTime) {
		TimeOfDay tod;

		std::time_t currentTime;
		
		if (unixTime > 0) {
			currentTime = static_cast<time_t>(unixTime);
		}
		else {
			// Get the current time
			auto now = std::chrono::system_clock::now();
			currentTime = std::chrono::system_clock::to_time_t(now);
		}

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
		else DHL = DailyHighsAndLows::Inside;

		if (comparisons[2]) LHL = LocalHighsAndLows::NLL;
		else if (comparisons[3]) LHL = LocalHighsAndLows::NLH;
		else LHL = LocalHighsAndLows::Inside;

		return { DHL, LHL };
	}

}