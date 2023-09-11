#include "AlertTags.h"

namespace Alerts {

	AlertTags::AlertTags(OptionType optType, TimeFrame timeFrame, RelativeToMoney rtm, TimeOfDay timeOfDay, VolumeStDev volStDev,
		VolumeThreshold volThreshold, PriceDelta underlyingPriceDelta, PriceDelta optionPriceDelta, DailyHighsAndLows underlyingDailyHL,
		LocalHighsAndLows underlyingLocalHL, DailyHighsAndLows optionDailyHL, LocalHighsAndLows optionLocalHL) :
		optType(optType), timeFrame(timeFrame), rtm(rtm), timeOfDay(timeOfDay), volStDev(volStDev), volThreshold(volThreshold),
		underlyingPriceDelta(underlyingPriceDelta), optionPriceDelta(optionPriceDelta), underlyingDailyHL(underlyingDailyHL),
		underlyingLocalHL(underlyingLocalHL), optionDailyHL(optionDailyHL), optionLocalHL(optionLocalHL) {}

	void AlertStats::updateAlertStats(double win, double percentWon) {
		total_++;
		totalWins_ += win;

		if (win > 0) {
			averageWin_ = averageWin_ + ((percentWon - averageWin_) / totalWins_);
		}

		winRate_ = totalWins_ / total_;
	}

	double AlertStats::winRate() { return winRate_; }
	double AlertStats::averageWin() { return averageWin_; }
	
	//==================================================
	// Alert Tag Stats
	//==================================================

	AlertStats::AlertStats() {}
	AlertStats::AlertStats(double totalWins, double total, double averageWin) : 
		totalWins_(totalWins), total_(total), averageWin_(averageWin) {
		
		winRate_ = totalWins_ / total_;
	}

	void AlertTagStats::updateStats(AlertTags tags, double win, double pctWon) {

		if (alertSpecificStats_.find(tags) != alertSpecificStats_.end()) {
			alertSpecificStats_[tags].updateAlertStats(win, pctWon);
		}
		else {
			AlertStats ast;
			ast.updateAlertStats(win, pctWon);
			alertSpecificStats_.insert({ tags, ast });
		}
			 
		updateStatsMap(optionTypeStats, tags.optType, win, pctWon);
		updateStatsMap(timeFrameStats, tags.timeFrame, win, pctWon);
		updateStatsMap(relativeToMoneyStats, tags.rtm, win, pctWon);
		updateStatsMap(timeoFDayStats, tags.timeOfDay, win, pctWon);
		updateStatsMap(volStDevStats, tags.volStDev, win, pctWon);
		updateStatsMap(volThresholdStats, tags.volThreshold, win, pctWon);
		updateStatsMap(underlyingPriceDeltaStats, tags.underlyingPriceDelta, win, pctWon);
		updateStatsMap(optionPriceDeltaStats, tags.optionPriceDelta, win, pctWon);
		updateStatsMap(uDailyHLStats, tags.underlyingDailyHL, win, pctWon);
		updateStatsMap(uLocalHLStats, tags.underlyingLocalHL, win, pctWon);
		updateStatsMap(oDailyHLStats, tags.optionDailyHL, win, pctWon);
		updateStatsMap(oLocalHLStats, tags.optionLocalHL, win, pctWon);
	}

	AlertStats AlertTagStats::alertSpecificStats(AlertTags tags) {
		if (alertSpecificStats_.find(tags) != alertSpecificStats_.end()) {
			return alertSpecificStats_.at(tags);
		}
		else {
			std::cout << "No data yet" << std::endl;
			return {};
		}
	}

	template<typename T>
	AlertStats AlertTagStats::tagSpecificStats(T tag) {
		return {};
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