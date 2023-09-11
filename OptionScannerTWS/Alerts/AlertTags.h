#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <unordered_map>
#include <map>

#include "../Enums.h"

namespace Alerts {

	struct AlertTags {
		OptionType optType;
		TimeFrame timeFrame;
		RelativeToMoney rtm;
		TimeOfDay timeOfDay;
		VolumeStDev volStDev;
		VolumeThreshold volThreshold;
		PriceDelta underlyingPriceDelta;
		PriceDelta optionPriceDelta;
		DailyHighsAndLows underlyingDailyHL;
		LocalHighsAndLows underlyingLocalHL;
		DailyHighsAndLows optionDailyHL;
		LocalHighsAndLows optionLocalHL;

		AlertTags(OptionType optType, TimeFrame timeFrame, RelativeToMoney rtm, TimeOfDay timeOfDay, VolumeStDev volStDev,
			VolumeThreshold volThreshold, PriceDelta underlyingPriceDelta, PriceDelta optionPriceDelta,
			DailyHighsAndLows underlyingDailyHL, LocalHighsAndLows underlyingLocalHL, DailyHighsAndLows optionDailyHL, LocalHighsAndLows optionLocalHL);
	};

	class AlertStats {
	public:
		// Initial constructor
		AlertStats();
		// Constructor for creation with db data
		AlertStats(double totalWins, double total, double averageWin);
		// A win will be considered 60% profit or more (2:1 ratio to stop loss)
		void updateAlertStats(double win, double percentWon);

		double winRate(); // 0 is a loss, 0.5 is break even or small win, 1 is 60% or more
		double averageWin();

	private:
		double winRate_{ 0 };
		double averageWin_{ 0 };

		double totalWins_{ 0 };

		double total_{ 0 };
	};

	// Hashing function for map with all alert tags
	struct AlertTagHash {
		std::size_t operator()(const AlertTags& tags) const;
	};

	class AlertTagStats {
	public:

		void updateStats(AlertTags tags, double win, double pctWon);

		AlertStats alertSpecificStats(AlertTags tags);
		
		template<typename T>
		AlertStats tagSpecificStats(T tag);

	private:

		// Contains data for all specific alert combinations
		std::map<AlertTags, AlertStats> alertSpecificStats_;

		// Individual tag stats
		std::unordered_map<OptionType, AlertStats> optionTypeStats;
		std::unordered_map<TimeFrame, AlertStats> timeFrameStats;
		std::unordered_map<RelativeToMoney, AlertStats> relativeToMoneyStats;
		std::unordered_map<TimeOfDay, AlertStats> timeoFDayStats;
		std::unordered_map<VolumeStDev, AlertStats> volStDevStats;
		std::unordered_map<VolumeThreshold, AlertStats> volThresholdStats;
		std::unordered_map<PriceDelta, AlertStats> underlyingPriceDeltaStats;
		std::unordered_map<PriceDelta, AlertStats> optionPriceDeltaStats;
		std::unordered_map<DailyHighsAndLows, AlertStats> uDailyHLStats;
		std::unordered_map<LocalHighsAndLows, AlertStats> uLocalHLStats;
		std::unordered_map<DailyHighsAndLows, AlertStats> oDailyHLStats;
		std::unordered_map<LocalHighsAndLows, AlertStats> oLocalHLStats;
	};

	template<typename T>
	void updateStatsMap(std::unordered_map<T, AlertStats>& statsMap, T key, double win, double pctWon);

	// Comparison operator to hold alertTags in a map
	bool operator<(const AlertTags& left, const AlertTags& right);
}