#include "Enums.h"

std::ostream& operator<<(std::ostream& out, const TimeFrame value) {
	return out << [value] {
#define PROCESS_VAL(p) case TimeFrame::p: return #p;
		switch (value) {
			PROCESS_VAL(FiveSecs);
			PROCESS_VAL(ThirtySecs);
			PROCESS_VAL(OneMin);
			PROCESS_VAL(FiveMin);
		default: return "";
		}
#undef PROCESS_VAL
	}();
}

std::string time_frame(TimeFrame val) {
	std::string res;
	switch (val)
	{
	case TimeFrame::FiveSecs:
		res = "FiveSecs";
		break;
	case TimeFrame::ThirtySecs:
		res = "ThirtySecs";
		break;
	case TimeFrame::OneMin:
		res = "OneMin";
		break;
	case TimeFrame::FiveMin:
		res = "FiveMin";
		break;
	default:
		break;
	}

	return res;
}

TimeFrame str_to_tf(const std::string& str) {
	if (str == "FiveSecs") return TimeFrame::FiveSecs;
	if (str == "ThirtySecs") return TimeFrame::ThirtySecs;
	if (str == "OneMin") return TimeFrame::OneMin;
	if (str == "FiveMin") return TimeFrame::FiveMin;

	throw std::invalid_argument("Unknown string for TimeFrame");
}

namespace Alerts {

	std::ostream& operator<<(std::ostream& out, const OptionType value) {
		return out << [value] {
	#define PROCESS_VAL(p) case OptionType::p: return #p;
			switch (value) {
				PROCESS_VAL(Call);
				PROCESS_VAL(Put);
			default: return "";
			}
	#undef PROCESS_VAL
		}();
	}

	std::ostream& operator<<(std::ostream& out, const RelativeToMoney value) {
		return out << [value] {
	#define PROCESS_VAL(p) case RelativeToMoney::p: return #p;
			switch (value) {
				PROCESS_VAL(ATM);
				PROCESS_VAL(ITM1);
				PROCESS_VAL(ITM2);
				PROCESS_VAL(ITM3);
				PROCESS_VAL(ITM4);
				PROCESS_VAL(DeepITM);
				PROCESS_VAL(OTM1);
				PROCESS_VAL(OTM2);
				PROCESS_VAL(OTM3);
				PROCESS_VAL(OTM4);
				PROCESS_VAL(DeepOTM);
			default: return "";
			}
	#undef PROCESS_VAL
		}();
	}

	std::ostream& operator<<(std::ostream& out, const TimeOfDay value) {
		return out << [value] {
	#define PROCESS_VAL(p) case TimeOfDay::p: return #p;
			switch (value) {
				PROCESS_VAL(Hour1);
				PROCESS_VAL(Hour2);
				PROCESS_VAL(Hour3);
				PROCESS_VAL(Hour4);
				PROCESS_VAL(Hour5);
				PROCESS_VAL(Hour6);
				PROCESS_VAL(Hour7);
			default: return "";
			}
	#undef PROCESS_VAL
		}();
	}

	std::ostream& operator<<(std::ostream& out, const VolumeStDev value) {
		return out << [value] {
	#define PROCESS_VAL(p) case VolumeStDev::p: return #p;
			switch (value) {
				PROCESS_VAL(Over1);
				PROCESS_VAL(Over2);
				PROCESS_VAL(Over3);
				PROCESS_VAL(Over4);
				PROCESS_VAL(LowVol);
			default: return "";
			}
	#undef PROCESS_VAL
		}();
	}

	std::ostream& operator<<(std::ostream& out, const VolumeThreshold value) {
		return out << [value] {
	#define PROCESS_VAL(p) case VolumeThreshold::p: return #p;
			switch (value) {
				PROCESS_VAL(Vol100);
				PROCESS_VAL(Vol250);
				PROCESS_VAL(Vol500);
				PROCESS_VAL(Vol1000);
				PROCESS_VAL(LowVol);
			default: return "";
			}
	#undef PROCESS_VAL
		}();
	}

	std::ostream& operator<<(std::ostream& out, const PriceDelta value) {
		return out << [value] {
	#define PROCESS_VAL(p) case PriceDelta::p: return #p;
			switch (value) {
				PROCESS_VAL(Under1);
				PROCESS_VAL(Under2);
				PROCESS_VAL(Over2);
			default: return "";
			}
	#undef PROCESS_VAL
		}();
	}

	std::ostream& operator<<(std::ostream& out, const DailyHighsAndLows value) {
		return out << [value] {
	#define PROCESS_VAL(p) case DailyHighsAndLows::p: return #p;
			switch (value) {
				PROCESS_VAL(NDL);
				PROCESS_VAL(NDH);
				PROCESS_VAL(Inside);
			default: return "";
			}
	#undef PROCESS_VAL
		}();
	}

	std::ostream& operator<<(std::ostream& out, const LocalHighsAndLows value) {
		return out << [value] {
	#define PROCESS_VAL(p) case LocalHighsAndLows::p: return #p;
			switch (value) {
				PROCESS_VAL(NLL);
				PROCESS_VAL(NLH);
				PROCESS_VAL(Inside);
			default: return "";
			}
	#undef PROCESS_VAL
		}();
	}

	//==========================================================
	// String Conversions
	//==========================================================

	std::string EnumString::option_type(OptionType val) {
		std::string res;
		switch (val)
		{
		case Alerts::OptionType::Call:
			res = "Call";
			break;
		case Alerts::OptionType::Put:
			res = "Put";
			break;
		default:
			break;
		}
		return res;
	}

	OptionType EnumString::str_to_option_type(const std::string& str) {
		if (str == "Call") return OptionType::Call;
		if (str == "Put") return OptionType::Put;

		throw std::invalid_argument("Unknown string for OptionType");
	}

	std::string EnumString::relative_to_money(RelativeToMoney val) {
		std::string res;
		switch (val)
		{
		case Alerts::RelativeToMoney::ATM:
			res = "ATM";
			break;
		case Alerts::RelativeToMoney::ITM1:
			res = "1 Strikes ITM";
			break;
		case Alerts::RelativeToMoney::ITM2:
			res = "2 Strikes ITM";
			break;
		case Alerts::RelativeToMoney::ITM3:
			res = "3 Strikes ITM";
			break;
		case Alerts::RelativeToMoney::ITM4:
			res = "4 Strikes ITM";
			break;
		case Alerts::RelativeToMoney::DeepITM:
			res = "Deep ITM";
			break;
		case Alerts::RelativeToMoney::OTM1:
			res = "1 Strikes OTM";
			break;
		case Alerts::RelativeToMoney::OTM2:
			res = "2 Strikes OTM";
			break;
		case Alerts::RelativeToMoney::OTM3:
			res = "3 Strikes OTM";
			break;
		case Alerts::RelativeToMoney::OTM4:
			res = "4 Strikes OTM";
			break;
		case Alerts::RelativeToMoney::DeepOTM:
			res = "Deep OTM";
			break;
		default:
			break;
		}
		return res;
	}

	RelativeToMoney EnumString::str_to_rtm(const std::string& str) {
		if (str == "ATM") return RelativeToMoney::ATM;
		if (str == "ITM1") return RelativeToMoney::ITM1;
		if (str == "ITM2") return RelativeToMoney::ITM2;
		if (str == "ITM3") return RelativeToMoney::ITM3;
		if (str == "ITM4") return RelativeToMoney::ITM4;
		if (str == "DeepITM") return RelativeToMoney::DeepITM;
		if (str == "OTM1") return RelativeToMoney::OTM1;
		if (str == "OTM2") return RelativeToMoney::OTM2;
		if (str == "OTM3") return RelativeToMoney::OTM3;
		if (str == "OTM4") return RelativeToMoney::OTM4;
		if (str == "DeepOTM") return RelativeToMoney::DeepOTM;

		throw std::invalid_argument("Unknown string for RTM");
	}

	std::string EnumString::time_of_day(TimeOfDay val) {
		std::string res;
		switch (val)
		{
		case Alerts::TimeOfDay::Hour1:
			res = "Hour1";
			break;
		case Alerts::TimeOfDay::Hour2:
			res = "Hour2";
			break;
		case Alerts::TimeOfDay::Hour3:
			res = "Hour3";
			break;
		case Alerts::TimeOfDay::Hour4:
			res = "Hour4";
			break;
		case Alerts::TimeOfDay::Hour5:
			res = "Hour5";
			break;
		case Alerts::TimeOfDay::Hour6:
			res = "Hour6";
			break;
		case Alerts::TimeOfDay::Hour7:
			res = "Hour7";
			break;
		default:
			break;
		}
		return res;
	}

	TimeOfDay EnumString::str_to_tod(const std::string& str) {
		if (str == "Hour1") return TimeOfDay::Hour1;
		if (str == "Hour2") return TimeOfDay::Hour2;
		if (str == "Hour3") return TimeOfDay::Hour3;
		if (str == "Hour4") return TimeOfDay::Hour4;
		if (str == "Hour5") return TimeOfDay::Hour5;
		if (str == "Hour6") return TimeOfDay::Hour6;
		if (str == "Hour7") return TimeOfDay::Hour7;

		throw std::invalid_argument("Unknown string for Time of Day");
	}

	std::string EnumString::vol_st_dev(VolumeStDev val) {
		std::string res;
		switch (val)
		{
		case Alerts::VolumeStDev::Over1:
			res = "Over1";
			break;
		case Alerts::VolumeStDev::Over2:
			res = "Over2";
			break;
		case Alerts::VolumeStDev::Over3:
			res = "Over3";
			break;
		case Alerts::VolumeStDev::Over4:
			res = "Over4";
			break;
		case Alerts::VolumeStDev::LowVol:
			res = "LowVol";
			break;
		default:
			break;
		}
		return res;
	}

	VolumeStDev EnumString::str_to_vol_stdev(const std::string& str) {
		if (str == "Over1") return VolumeStDev::Over1;
		if (str == "Over2") return VolumeStDev::Over2;
		if (str == "Over3") return VolumeStDev::Over3;
		if (str == "Over4") return VolumeStDev::Over4;
		if (str == "LowVol") return VolumeStDev::LowVol;

		throw std::invalid_argument("Unknown string for Vol StDev");
	}

	std::string EnumString::vol_threshold(VolumeThreshold val) {
		std::string res;
		switch (val)
		{
		case Alerts::VolumeThreshold::Vol100:
			res = "VolOver100";
			break;
		case Alerts::VolumeThreshold::Vol250:
			res = "VolOver250";
			break;
		case Alerts::VolumeThreshold::Vol500:
			res = "VolOver500";
			break;
		case Alerts::VolumeThreshold::Vol1000:
			res = "VolOver1000";
			break;
		case Alerts::VolumeThreshold::LowVol:
			res = "LowVol";
			break;
		default:
			break;
		}
		return res;
	}

	VolumeThreshold EnumString::str_to_vol_thresh(const std::string& str) {
		if (str == "VolOver100") return VolumeThreshold::Vol100;
		if (str == "VolOver250") return VolumeThreshold::Vol250;
		if (str == "VolOver500") return VolumeThreshold::Vol500;
		if (str == "VolOver1000") return VolumeThreshold::Vol1000;
		if (str == "LowVol") return VolumeThreshold::LowVol;

		throw std::invalid_argument("Unknown string for Vol Threshold");
	}


	std::string EnumString::price_delta(PriceDelta val) {
		std::string res;
		switch (val)
		{
		case Alerts::PriceDelta::Under1:
			res = "Under1";
			break;
		case Alerts::PriceDelta::Under2:
			res = "Under2";
			break;
		case Alerts::PriceDelta::Over2:
			res = "Over2";
			break;
		default:
			break;
		}
		return res;
	}

	PriceDelta EnumString::str_to_price_delta(const std::string& str) {
		if (str == "Under1") return PriceDelta::Under1;
		if (str == "Under2") return PriceDelta::Under2;
		if (str == "Over2") return PriceDelta::Over2;

		throw std::invalid_argument("Unknown string for Price Delta");
	}

	std::string EnumString::daily_highs_and_lows(DailyHighsAndLows val) {
		std::string res;
		switch (val)
		{
		case Alerts::DailyHighsAndLows::NDL:
			res = "NearDailyLow";
			break;
		case Alerts::DailyHighsAndLows::NDH:
			res = "NearDailyHigh";
			break;
		case Alerts::DailyHighsAndLows::Inside:
			res = "InsideRange";
			break;
		default:
			break;
		}
		return res;
	}

	DailyHighsAndLows EnumString::str_to_daily_hl(const std::string& str) {
		if (str == "NearDailyLow") return DailyHighsAndLows::NDL;
		if (str == "NearDailyHigh") return DailyHighsAndLows::NDH;
		if (str == "InsideRange") return DailyHighsAndLows::Inside;

		throw std::invalid_argument("Unknown string for Daily Highs and Lows");
	}

	std::string EnumString::local_highs_and_lows(LocalHighsAndLows val) {
		std::string res;
		switch (val)
		{
		case Alerts::LocalHighsAndLows::NLL:
			res = "NearLocalLow";
			break;
		case Alerts::LocalHighsAndLows::NLH:
			res = "NearLocalHigh";
			break;
		case Alerts::LocalHighsAndLows::Inside:
			res = "InsideRange";
			break;
		default:
			break;
		}
		return res;
	}

	LocalHighsAndLows EnumString::str_to_local_hl(const std::string& str) {
		if (str == "NearLocalLow") return LocalHighsAndLows::NLL;
		if (str == "NearLocalHigh") return LocalHighsAndLows::NLH;
		if (str == "InsideRange") return LocalHighsAndLows::Inside;

		throw std::invalid_argument("Unknown string for Local Highs and Lows");
	}

	std::string EnumString::tag_category(TagCategory val) {
		std::string res;
		switch (val)
		{
		case Alerts::TagCategory::OptionType:
			res = "OptionType";
			break;
		case Alerts::TagCategory::TimeFrame:
			res = "TimeFrame";
			break;
		case Alerts::TagCategory::RelativeToMoney:
			res = "RelativeToMoney";
			break;
		case Alerts::TagCategory::VolumeStDev:
			res = "VolumeStDev";
			break;
		case Alerts::TagCategory::VolumeThreshold:
			res = "VolumeThreshold";
			break;
		case Alerts::TagCategory::UnderlyingPriceDelta:
			res = "UnderlyingPriceDelta";
			break;
		case Alerts::TagCategory::OptionPriceDelta:
			res = "OptionPriceDelta";
			break;
		case Alerts::TagCategory::UnderlyingDailyHighsAndLows:
			res = "UnderlyingDailyHighsAndLows";
			break;
		case Alerts::TagCategory::OptionDailyHighsAndLows:
			res = "OptionDailyHighsAndLows";
			break;
		case Alerts::TagCategory::UnderlyingLocalHighsAndLows:
			res = "UnderlyingLocalHighsAndLows";
			break;
		case Alerts::TagCategory::OptionLocalHighsAndLows:
			res = "OptionLocalHighsAndLows";
			break;
		default:
			break;
		}
		return res;
	}

	TagCategory EnumString::str_to_tag_category(const std::string& str) {
		if (str == "OptionType") return TagCategory::OptionType;
		if (str == "TimeFrame") return TagCategory::TimeFrame;
		if (str == "RelativeToMoney") return TagCategory::RelativeToMoney;
		if (str == "VolumeStDev") return TagCategory::VolumeStDev;
		if (str == "VolumeThreshold") return TagCategory::VolumeThreshold;
		if (str == "UnderlyingPriceDelta") return TagCategory::UnderlyingPriceDelta;
		if (str == "OptionPriceDelta") return TagCategory::OptionPriceDelta;
		if (str == "UnderlyingDailyHighsAndLows") return TagCategory::UnderlyingDailyHighsAndLows;
		if (str == "OptionDailyHighsAndLows") return TagCategory::OptionDailyHighsAndLows;
		if (str == "UnderlyingLocalHighsAndLows") return TagCategory::UnderlyingLocalHighsAndLows;
		if (str == "OptionLocalHighsAndLows") return TagCategory::OptionLocalHighsAndLows;

		throw std::invalid_argument("Unknown string for Tag Category");
	}

	std::unordered_map<std::pair<std::string, std::string>, int, PairHash> TagDBInterface::tagToInt = {

		{ {Alerts::EnumString::option_type(Alerts::OptionType::Call), EnumString::tag_category(TagCategory::OptionType)}, 1 },
		{ {Alerts::EnumString::option_type(Alerts::OptionType::Put), EnumString::tag_category(TagCategory::OptionType)}, 2 },

		{ {time_frame(TimeFrame::FiveSecs), EnumString::tag_category(TagCategory::TimeFrame)}, 3 },
		{ {time_frame(TimeFrame::ThirtySecs), EnumString::tag_category(TagCategory::TimeFrame)}, 4 },
		{ {time_frame(TimeFrame::OneMin), EnumString::tag_category(TagCategory::TimeFrame)}, 5 },
		{ {time_frame(TimeFrame::FiveMin), EnumString::tag_category(TagCategory::TimeFrame)}, 6 },

		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ATM), EnumString::tag_category(TagCategory::RelativeToMoney)}, 7 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ITM1), EnumString::tag_category(TagCategory::RelativeToMoney)}, 8 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ITM2), EnumString::tag_category(TagCategory::RelativeToMoney)}, 9 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ITM3), EnumString::tag_category(TagCategory::RelativeToMoney)}, 10 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ITM4), EnumString::tag_category(TagCategory::RelativeToMoney)}, 11 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::DeepITM), EnumString::tag_category(TagCategory::RelativeToMoney)}, 12 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::OTM1), EnumString::tag_category(TagCategory::RelativeToMoney)}, 13 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::OTM2), EnumString::tag_category(TagCategory::RelativeToMoney)}, 14 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::OTM3), EnumString::tag_category(TagCategory::RelativeToMoney)}, 15 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::OTM4), EnumString::tag_category(TagCategory::RelativeToMoney)}, 16 },
		{ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::DeepOTM), EnumString::tag_category(TagCategory::RelativeToMoney)}, 17 },

		{ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour1), EnumString::tag_category(TagCategory::TimeOfDay)}, 18 },
		{ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour2), EnumString::tag_category(TagCategory::TimeOfDay)}, 19 },
		{ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour3), EnumString::tag_category(TagCategory::TimeOfDay)}, 20 },
		{ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour4), EnumString::tag_category(TagCategory::TimeOfDay)}, 21 },
		{ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour5), EnumString::tag_category(TagCategory::TimeOfDay)}, 22 },
		{ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour6), EnumString::tag_category(TagCategory::TimeOfDay)}, 23 },
		{ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour7), EnumString::tag_category(TagCategory::TimeOfDay)}, 24 },

		{ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::Over1), EnumString::tag_category(TagCategory::VolumeStDev)}, 25 },
		{ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::Over2), EnumString::tag_category(TagCategory::VolumeStDev)}, 26 },
		{ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::Over3), EnumString::tag_category(TagCategory::VolumeStDev)}, 27 },
		{ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::Over4), EnumString::tag_category(TagCategory::VolumeStDev)}, 28 },
		{ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::LowVol), EnumString::tag_category(TagCategory::VolumeStDev)}, 29 },

		{ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::Vol100), EnumString::tag_category(TagCategory::VolumeThreshold)}, 30 },
		{ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::Vol250), EnumString::tag_category(TagCategory::VolumeThreshold)}, 31 },
		{ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::Vol500), EnumString::tag_category(TagCategory::VolumeThreshold)}, 32 },
		{ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::Vol1000), EnumString::tag_category(TagCategory::VolumeThreshold)}, 33 },
		{ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::LowVol), EnumString::tag_category(TagCategory::VolumeThreshold)}, 34 },

		{ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Under1), EnumString::tag_category(TagCategory::UnderlyingPriceDelta)}, 35 },
		{ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Under2),EnumString::tag_category(TagCategory::UnderlyingPriceDelta)}, 36 },
		{ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Over2), EnumString::tag_category(TagCategory::UnderlyingPriceDelta)}, 37 },

		{ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Under1), EnumString::tag_category(TagCategory::OptionPriceDelta)}, 38 },
		{ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Under2), EnumString::tag_category(TagCategory::OptionPriceDelta)}, 39 },
		{ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Over2), EnumString::tag_category(TagCategory::OptionPriceDelta)}, 40 },

		{ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::NDL), EnumString::tag_category(TagCategory::UnderlyingDailyHighsAndLows)}, 41 },
		{ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::NDH), EnumString::tag_category(TagCategory::UnderlyingDailyHighsAndLows)}, 42 },
		{ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::Inside), EnumString::tag_category(TagCategory::UnderlyingDailyHighsAndLows)}, 43 },

		{ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::NDL), EnumString::tag_category(TagCategory::OptionDailyHighsAndLows)}, 44 },
		{ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::NDH), EnumString::tag_category(TagCategory::OptionDailyHighsAndLows)}, 45 },
		{ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::Inside), EnumString::tag_category(TagCategory::OptionDailyHighsAndLows)}, 46 },

		{ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::NLL), EnumString::tag_category(TagCategory::UnderlyingLocalHighsAndLows)}, 47 },
		{ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::NLH), EnumString::tag_category(TagCategory::UnderlyingLocalHighsAndLows)}, 48 },
		{ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::Inside), EnumString::tag_category(TagCategory::UnderlyingLocalHighsAndLows)}, 49 },

		{ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::NLL), EnumString::tag_category(TagCategory::OptionLocalHighsAndLows)}, 50 },
		{ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::NLH), EnumString::tag_category(TagCategory::OptionLocalHighsAndLows)}, 51 },
		{ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::Inside), EnumString::tag_category(TagCategory::OptionLocalHighsAndLows)}, 52 }
	};

	std::unordered_map<int, std::pair<std::string, std::string>> TagDBInterface::intToTag;

	void TagDBInterface::initialize() {
		// Fill the intToTagMap
		for (const auto& key : tagToInt) {
			intToTag[key.second] = key.first;
		}
	}

	StaticInitializer staticInitializerInstance;
}
