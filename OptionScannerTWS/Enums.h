#pragma once

#include <iostream>
#include <unordered_map>

// All enumerations used alongside price and options data with output overloads

// Time frame enums
enum class TimeFrame {
	FiveSecs,
	ThirtySecs,
	OneMin,
	FiveMin
};

std::ostream& operator<<(std::ostream& out, const TimeFrame value);
std::string time_frame(TimeFrame val);

// Enums attached to alerts
namespace Alerts {

	// Call or Put
	enum class OptionType { Call, Put };

	// In the Money or Out of the Money
	enum class RelativeToMoney {
		ATM,
		ITM1, ITM2, ITM3, ITM4, DeepITM,
		OTM1, OTM2, OTM3, OTM4, DeepOTM
	};

	// Time of day
	enum class TimeOfDay { Hour1, Hour2, Hour3, Hour4, Hour5, Hour6, Hour7 };

	// Volume Data
	enum class VolumeStDev { Over1, Over2, Over3, Over4, LowVol };
	enum class VolumeThreshold { Vol100, Vol250, Vol500, Vol1000, LowVol };

	// Price delta for accuracy in probability
	enum class PriceDelta { Under1, Under2, Over2 };

	// Contract and Underlying daily highs and lows
	enum class DailyHighsAndLows { NDL, NDH, Inside }; // Near daily high, near daily low
	enum class LocalHighsAndLows { NLL, NLH, Inside }; // Near local low, near local high

	// Additions for later
	// Repeated hits (more than 2 of same alert back to back)
	// Repeated hits (more than 3)
	// Repeated hits (more than 5)

	// Higher than average cumulative vol

	// Low local high - local low delta
	// High local high - local low delta
	// Low daily high - daily low delta
	// High daily high - daily low delta

	// Overwrite functions for iostream usage
	std::ostream& operator<<(std::ostream& out, const OptionType value);
	std::ostream& operator<<(std::ostream& out, const RelativeToMoney value);
	std::ostream& operator<<(std::ostream& out, const TimeOfDay value);
	std::ostream& operator<<(std::ostream& out, const VolumeStDev value);
	std::ostream& operator<<(std::ostream& out, const VolumeThreshold value);
	std::ostream& operator<<(std::ostream& out, const PriceDelta value);
	std::ostream& operator<<(std::ostream& out, const DailyHighsAndLows value);
	std::ostream& operator<<(std::ostream& out, const LocalHighsAndLows value);


	// Due to the nature of spdlog, it has conflicting format issues with the ostream overriden functions
	// Manually creating string conversions for each enum class
	class EnumString {
	public:
		static std::string option_type(OptionType val);
		static std::string relative_to_money(RelativeToMoney val);
		static std::string time_of_day(TimeOfDay val);
		static std::string vol_st_dev(VolumeStDev val);
		static std::string vol_threshold(VolumeThreshold val);
		static std::string price_delta(PriceDelta val);
		static std::string daily_highs_and_lows(DailyHighsAndLows val);
		static std::string local_highs_and_lows(LocalHighsAndLows val);
	};
}