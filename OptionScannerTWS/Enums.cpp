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
}
