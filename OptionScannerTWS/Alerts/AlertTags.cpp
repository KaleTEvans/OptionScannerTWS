#include "AlertTags.h"

namespace Alerts {

	AlertTags::AlertTags(OptionType optType, TimeFrame timeFrame, RelativeToMoney rtm, TimeOfDay timeOfDay, VolumeStDev volStDev,
		VolumeThreshold volThreshold, PriceDelta underlyingPriceDelta, PriceDelta optionPriceDelta, DailyHighsAndLows underlyingDailyHL,
		LocalHighsAndLows underlyingLocalHL, DailyHighsAndLows optionDailyHL, LocalHighsAndLows optionLocalHL) :
		optType(optType), timeFrame(timeFrame), rtm(rtm), timeOfDay(timeOfDay), volStDev(volStDev), volThreshold(volThreshold),
		underlyingPriceDelta(underlyingPriceDelta), optionPriceDelta(optionPriceDelta), underlyingDailyHL(underlyingDailyHL),
		underlyingLocalHL(underlyingLocalHL), optionDailyHL(optionDailyHL), optionLocalHL(optionLocalHL) {}

	//==================================
	// Alert Stats
	//==================================

	AlertStats::AlertStats() {}

	AlertStats::AlertStats(double weightedWins, double unweightedWins, double total, double averageWin) :
		weightedWins_(weightedWins), unweightedWins_(unweightedWins), total_(total), averageWin_(averageWin) {

		winRate_ = weightedWins_ / total_;
	}

	void AlertStats::updateAlertStats(const double win, const double percentWon) {
		total_++;
		weightedWins_ += win;

		if (win > 0) {
			unweightedWins_++;
			averageWin_ = averageWin_ + ((percentWon - averageWin_) / unweightedWins_);
		}

		winRate_ = weightedWins_ / total_;
	}

	double AlertStats::winRate() const { return winRate_; }
	double AlertStats::averageWin() const { return averageWin_; }
	double AlertStats::totalAlerts() const { return total_; }
	
	//==================================================
	// Alert Tag Stats
	//==================================================

	void AlertTagStats::updateStats(AlertTags tags, double win, double pctWon) {

		if (alertSpecificStats_.find(tags) != alertSpecificStats_.end()) {
			alertSpecificStats_[tags].updateAlertStats(win, pctWon);
		}
		else {
			AlertStats ast;
			ast.updateAlertStats(win, pctWon);
			alertSpecificStats_.insert({ tags, ast });
		}
			 
		updateStatsMap(optionTypeStats_, tags.optType, win, pctWon);
		updateStatsMap(timeFrameStats_, tags.timeFrame, win, pctWon);
		updateStatsMap(relativeToMoneyStats_, tags.rtm, win, pctWon);
		updateStatsMap(timeOfDayStats_, tags.timeOfDay, win, pctWon);
		updateStatsMap(volStDevStats_, tags.volStDev, win, pctWon);
		updateStatsMap(volThresholdStats_, tags.volThreshold, win, pctWon);
		updateStatsMap(underlyingPriceDeltaStats_, tags.underlyingPriceDelta, win, pctWon);
		updateStatsMap(optionPriceDeltaStats_, tags.optionPriceDelta, win, pctWon);
		updateStatsMap(uDailyHLStats_, tags.underlyingDailyHL, win, pctWon);
		updateStatsMap(uLocalHLStats_, tags.underlyingLocalHL, win, pctWon);
		updateStatsMap(oDailyHLStats_, tags.optionDailyHL, win, pctWon);
		updateStatsMap(oLocalHLStats_, tags.optionLocalHL, win, pctWon);
	}

	AlertStats AlertTagStats::alertSpecificStats(AlertTags tags) {
		if (alertSpecificStats_.find(tags) == alertSpecificStats_.end()) {
			// If no data, insert a class with 0 values
			AlertStats ast;
			alertSpecificStats_.insert({ tags, ast });
		}

		return alertSpecificStats_.at(tags);
	}

	// Alert type accessors
	AlertStats AlertTagStats::optionTypeStats(OptionType key) { return checkStatsMap(key, optionTypeStats_); }
	AlertStats AlertTagStats::timeFrameStats(TimeFrame key) { return checkStatsMap(key, timeFrameStats_); }
	AlertStats AlertTagStats::relativeToMoneyStats(RelativeToMoney key) { return checkStatsMap(key, relativeToMoneyStats_); }
	AlertStats AlertTagStats::timeOfDayStats(TimeOfDay key) { return checkStatsMap(key, timeOfDayStats_); }
	AlertStats AlertTagStats::volStDevStats(VolumeStDev key) { return checkStatsMap(key, volStDevStats_); }
	AlertStats AlertTagStats::volThresholdStats(VolumeThreshold key) { return checkStatsMap(key, volThresholdStats_); }
	AlertStats AlertTagStats::underlyingDeltaStats(PriceDelta key) { return checkStatsMap(key, underlyingPriceDeltaStats_); }
	AlertStats AlertTagStats::optionDeltaStats(PriceDelta key) { return checkStatsMap(key, optionPriceDeltaStats_); }
	AlertStats AlertTagStats::underlyingDailyHLStats(DailyHighsAndLows key) { return checkStatsMap(key, uDailyHLStats_); }
	AlertStats AlertTagStats::underlyingLocalHLStats(LocalHighsAndLows key) { return checkStatsMap(key, uLocalHLStats_); }
	AlertStats AlertTagStats::optionDailyHLStats(DailyHighsAndLows key) { return checkStatsMap(key, oDailyHLStats_); }
	AlertStats AlertTagStats::optionLocalHLStats(LocalHighsAndLows key) { return checkStatsMap(key, oLocalHLStats_); }

	void AlertTagStats::logAllTagStats() {
#ifndef TEST_CONFIG

		OPTIONSCANNER_INFO("Beginning Tag Stat Output ====================================================================");
		OPTIONSCANNER_INFO("OptionType | Call | Win Rate: {} | Average Win: {}",
			optionTypeStats(OptionType::Call).winRate(), optionTypeStats(OptionType::Call).averageWin());
		OPTIONSCANNER_INFO("OptionType | Put | Win Rate: {} | Average Win: {}",
			optionTypeStats(OptionType::Put).winRate(), optionTypeStats(OptionType::Put).averageWin());

		OPTIONSCANNER_INFO("TimeFrame | 5 Sec | Win Rate: {} | Average Win: {}",
			timeFrameStats(TimeFrame::FiveSecs).winRate(), timeFrameStats(TimeFrame::FiveSecs).averageWin());
		OPTIONSCANNER_INFO("TimeFrame | 30 Sec | Win Rate: {} | Average Win: {}",
			timeFrameStats(TimeFrame::ThirtySecs).winRate(), timeFrameStats(TimeFrame::ThirtySecs).averageWin());
		OPTIONSCANNER_INFO("TimeFrame | 1 Min | Win Rate: {} | Average Win: {}",
			timeFrameStats(TimeFrame::OneMin).winRate(), timeFrameStats(TimeFrame::OneMin).averageWin());
		OPTIONSCANNER_INFO("TimeFrame | 5 Min | Win Rate: {} | Average Win: {}",
			timeFrameStats(TimeFrame::FiveMin).winRate(), timeFrameStats(TimeFrame::FiveMin).averageWin());

		OPTIONSCANNER_INFO("RTM | ATM | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::ATM).winRate(), relativeToMoneyStats(RelativeToMoney::ATM).averageWin());
		OPTIONSCANNER_INFO("RTM | ITM1 | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::ITM1).winRate(), relativeToMoneyStats(RelativeToMoney::ITM1).averageWin());
		OPTIONSCANNER_INFO("RTM | ITM2 | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::ITM2).winRate(), relativeToMoneyStats(RelativeToMoney::ITM2).averageWin());
		OPTIONSCANNER_INFO("RTM | ITM3 | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::ITM3).winRate(), relativeToMoneyStats(RelativeToMoney::ITM3).averageWin());
		OPTIONSCANNER_INFO("RTM | ITM4 | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::ITM4).winRate(), relativeToMoneyStats(RelativeToMoney::ITM4).averageWin());
		OPTIONSCANNER_INFO("RTM | Deep ITM | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::DeepITM).winRate(), relativeToMoneyStats(RelativeToMoney::DeepITM).averageWin());
		OPTIONSCANNER_INFO("RTM | OTM1 | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::OTM1).winRate(), relativeToMoneyStats(RelativeToMoney::OTM1).averageWin());
		OPTIONSCANNER_INFO("RTM | OTM2 | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::OTM2).winRate(), relativeToMoneyStats(RelativeToMoney::OTM2).averageWin());
		OPTIONSCANNER_INFO("RTM | OTM3 | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::OTM3).winRate(), relativeToMoneyStats(RelativeToMoney::OTM3).averageWin());
		OPTIONSCANNER_INFO("RTM | OTM4 | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::OTM4).winRate(), relativeToMoneyStats(RelativeToMoney::OTM4).averageWin());
		OPTIONSCANNER_INFO("RTM | Deep OTM | Win Rate: {} | Average Win: {}",
			relativeToMoneyStats(RelativeToMoney::DeepOTM).winRate(), relativeToMoneyStats(RelativeToMoney::DeepOTM).averageWin());

		OPTIONSCANNER_INFO("VolumeStDev | Over1 | Win Rate: {} | Average Win: {}",
			volStDevStats(VolumeStDev::Over1).winRate(), volStDevStats(VolumeStDev::Over1).averageWin());
		OPTIONSCANNER_INFO("VolumeStDev | Over2 | Win Rate: {} | Average Win: {}",
			volStDevStats(VolumeStDev::Over2).winRate(), volStDevStats(VolumeStDev::Over2).averageWin());
		OPTIONSCANNER_INFO("VolumeStDev | Over3 | Win Rate: {} | Average Win: {}",
			volStDevStats(VolumeStDev::Over3).winRate(), volStDevStats(VolumeStDev::Over3).averageWin());
		OPTIONSCANNER_INFO("VolumeStDev | Over4 | Win Rate: {} | Average Win: {}",
			volStDevStats(VolumeStDev::Over4).winRate(), volStDevStats(VolumeStDev::Over4).averageWin());
		OPTIONSCANNER_INFO("VolumeStDev | Low Vol | Win Rate: {} | Average Win: {}",
			volStDevStats(VolumeStDev::LowVol).winRate(), volStDevStats(VolumeStDev::LowVol).averageWin());

		OPTIONSCANNER_INFO("Volume Threshold | Over100 | Win Rate: {} | Average Win: {}",
			volThresholdStats(VolumeThreshold::Vol100).winRate(), volThresholdStats(VolumeThreshold::Vol100).averageWin());
		OPTIONSCANNER_INFO("Volume Threshold | Over250 | Win Rate: {} | Average Win: {}",
			volThresholdStats(VolumeThreshold::Vol250).winRate(), volThresholdStats(VolumeThreshold::Vol250).averageWin());
		OPTIONSCANNER_INFO("Volume Threshold | Over500 | Win Rate: {} | Average Win: {}",
			volThresholdStats(VolumeThreshold::Vol500).winRate(), volThresholdStats(VolumeThreshold::Vol500).averageWin());
		OPTIONSCANNER_INFO("Volume Threshold | Over1000 | Win Rate: {} | Average Win: {}",
			volThresholdStats(VolumeThreshold::Vol1000).winRate(), volThresholdStats(VolumeThreshold::Vol1000).averageWin());
		OPTIONSCANNER_INFO("Volume Threshold | Low Vol | Win Rate: {} | Average Win: {}",
			volThresholdStats(VolumeThreshold::LowVol).winRate(), volThresholdStats(VolumeThreshold::LowVol).averageWin());

		OPTIONSCANNER_INFO("Ending stat output ====================================================================");

#endif // !TEST_CONFIG
	}

	template<typename T>
	AlertStats AlertTagStats::checkStatsMap(T key, std::unordered_map<T, AlertStats>& statsMap) {
		if (statsMap.find(key) == statsMap.end()) {
			AlertStats ast;
			statsMap.insert({ key, ast });
		}

		return statsMap.at(key);
	}

	template<typename T>
	void updateStatsMap(std::unordered_map<T, AlertStats>& statsMap, T key, double win, double pctWon) {
		if (statsMap.find(key) != statsMap.end()) {
			statsMap[key].updateAlertStats(win, pctWon);
		}
		else {
			AlertStats ast;
			ast.updateAlertStats(win, pctWon);
			statsMap.insert({ key, ast });
		}
	}

	bool operator<(const AlertTags& left, const AlertTags& right) {
		if (left.optType < right.optType) return true;
		if (right.optType < left.optType) return false;

		if (left.timeFrame < right.timeFrame) return true;
		if (right.timeFrame < left.timeFrame) return false;

		if (left.rtm < right.rtm) return true;
		if (right.rtm < left.rtm) return false;

		if (left.timeOfDay < right.timeOfDay) return true;
		if (right.timeOfDay < left.timeOfDay) return false;

		if (left.volStDev < right.volStDev) return true;
		if (right.volStDev < left.volStDev) return false;

		if (left.volThreshold < right.volThreshold) return true;
		if (right.volThreshold < left.volThreshold) return false;

		if (left.underlyingPriceDelta < right.underlyingPriceDelta) return true;
		if (right.underlyingPriceDelta < left.underlyingPriceDelta) return false;

		if (left.optionPriceDelta < right.optionPriceDelta) return true;
		if (right.optionPriceDelta < left.optionPriceDelta) return false;

		if (left.underlyingDailyHL < right.underlyingDailyHL) return true;
		if (right.underlyingDailyHL < left.underlyingDailyHL) return false;

		if (left.underlyingLocalHL < right.underlyingLocalHL) return true;
		if (right.underlyingLocalHL < left.underlyingLocalHL) return false;

		if (left.optionDailyHL < right.optionDailyHL) return true;
		if (right.optionDailyHL < left.optionDailyHL) return false;

		if (left.optionLocalHL < right.optionLocalHL) return true;
		if (right.optionLocalHL < left.optionLocalHL) return false;

		return false; // If all numbers are equal, return false
	}
}