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

namespace Alerts {

	std::ostream& operator<<(std::ostream& out, const RelativeToMoney value) {
		return out << [value] {
	#define PROCESS_VAL(p) case RelativeToMoney::p: return #p;
			switch (value) {
				PROCESS_VAL(ATM);
				PROCESS_VAL(ITM1);
				PROCESS_VAL(ITM2);
				PROCESS_VAL(ITM3);
				PROCESS_VAL(ITM4);
				PROCESS_VAL(ITM5);
				PROCESS_VAL(OTM1);
				PROCESS_VAL(OTM2);
				PROCESS_VAL(OTM3);
				PROCESS_VAL(OTM4);
				PROCESS_VAL(OTM5);
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
			default: return "";
			}
	#undef PROCESS_VAL
		}();
	}
}