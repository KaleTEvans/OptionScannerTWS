#define _CRT_SECURE_NO_WARNINGS

#define TEST_CONFIG

#pragma once

#include <unordered_map>
#include <map>

#include "../Enums.h"

#ifndef TEST_CONFIG
#include "../Logger.h"
#endif // !TEST_CONFIG

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
		AlertStats(double weightedWins, double unweightedWins, double total, double averageWin);
		// A win will be considered 60% profit or more (2:1 ratio to stop loss)
		void updateAlertStats(const double win, const double percentWon);

		double winRate() const; // 0 is a loss, 0.5 is break even or small win, 1 is 60% or more
		double averageWin() const;
		double totalAlerts() const;

	private:
		double winRate_{ 0 };
		double averageWin_{ 0 };

		double unweightedWins_{ 0 };
		double weightedWins_{ 0 };

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
		
		// Alert type accessors
		AlertStats optionTypeStats(OptionType key);
		AlertStats timeFrameStats(TimeFrame key);
		AlertStats relativeToMoneyStats(RelativeToMoney key);
		AlertStats timeOfDayStats(TimeOfDay key);
		AlertStats volStDevStats(VolumeStDev key);
		AlertStats volThresholdStats(VolumeThreshold key);
		AlertStats underlyingDeltaStats(PriceDelta key);
		AlertStats optionDeltaStats(PriceDelta key);
		AlertStats underlyingDailyHLStats(DailyHighsAndLows key);
		AlertStats underlyingLocalHLStats(LocalHighsAndLows key);
		AlertStats optionDailyHLStats(DailyHighsAndLows key);
		AlertStats optionLocalHLStats(LocalHighsAndLows key);

		void logAllTagStats();

		// This helper template will simplify checking if a map has a value or not
		template<typename T>
		AlertStats checkStatsMap(T key, std::unordered_map<T, AlertStats>& statsMap);

	private:

		// Contains data for all specific alert combinations
		std::map<AlertTags, AlertStats> alertSpecificStats_;

		// Individual tag stats
		std::unordered_map<OptionType, AlertStats> optionTypeStats_;
		std::unordered_map<TimeFrame, AlertStats> timeFrameStats_;
		std::unordered_map<RelativeToMoney, AlertStats> relativeToMoneyStats_;
		std::unordered_map<TimeOfDay, AlertStats> timeOfDayStats_;
		std::unordered_map<VolumeStDev, AlertStats> volStDevStats_;
		std::unordered_map<VolumeThreshold, AlertStats> volThresholdStats_;
		std::unordered_map<PriceDelta, AlertStats> underlyingPriceDeltaStats_;
		std::unordered_map<PriceDelta, AlertStats> optionPriceDeltaStats_;
		std::unordered_map<DailyHighsAndLows, AlertStats> uDailyHLStats_;
		std::unordered_map<LocalHighsAndLows, AlertStats> uLocalHLStats_;
		std::unordered_map<DailyHighsAndLows, AlertStats> oDailyHLStats_;
		std::unordered_map<LocalHighsAndLows, AlertStats> oLocalHLStats_;
	};

	template<typename T>
	void updateStatsMap(std::unordered_map<T, AlertStats>& statsMap, T key, double win, double pctWon);

	// Comparison operator to hold alertTags in a map
	bool operator<(const AlertTags& left, const AlertTags& right);
}