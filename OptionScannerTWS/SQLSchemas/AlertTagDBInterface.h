#pragma once

#include <memory>
#include <unordered_map>


#include "../Alerts/AlertTags.h" 

using std::string;

namespace OptionDB {

	namespace AlertTables {
		struct PairHash {
			template <class T1, class T2>
			std::size_t operator() (const std::pair<T1, T2>& p) const {
				auto h1 = std::hash<T1>{}(p.first);
				auto h2 = std::hash<T2>{}(p.second);
				return h1 ^ h2;
			}
		};


		class AlertTagDBInterface {
		public:
			AlertTagDBInterface() { initialize(); }

			std::unordered_map<std::pair<string, string>, int, PairHash> tagInterface() { return tagDBInterface; }

			std::vector<int> convertAlert(Alerts::AlertTags tags) {
				std::vector<int> tagID;

				std::pair<string, string> p1{ Alerts::EnumString::option_type(tags.optType), "OptionType" };
				std::pair<string, string> p2{ time_frame(tags.timeFrame), "TimeFrame" };
				std::pair<string, string> p3{ Alerts::EnumString::relative_to_money(tags.rtm), "RelativeToMoney" };
				std::pair<string, string> p4{ Alerts::EnumString::time_of_day(tags.timeOfDay), "TimeOfDay" };
				std::pair<string, string> p5{ Alerts::EnumString::vol_st_dev(tags.volStDev), "VolumeStDev" };
				std::pair<string, string> p6{ Alerts::EnumString::vol_threshold(tags.volThreshold), "VolumeThreshold" };
				std::pair<string, string> p7{ Alerts::EnumString::price_delta(tags.underlyingPriceDelta), "UnderlyingPriceDelta" };
				std::pair<string, string> p8{ Alerts::EnumString::price_delta(tags.optionPriceDelta), "OptionPriceDelta" };
				std::pair<string, string> p9{ Alerts::EnumString::daily_highs_and_lows(tags.underlyingDailyHL), "UnderlyingDailyHighsAndLows" };
				std::pair<string, string> p10{ Alerts::EnumString::daily_highs_and_lows(tags.optionDailyHL), "OptionDailyHighsAndLows" };
				std::pair<string, string> p11{ Alerts::EnumString::local_highs_and_lows(tags.underlyingLocalHL), "UnderlyingLocalHighsAndLows" };
				std::pair<string, string> p12{ Alerts::EnumString::local_highs_and_lows(tags.optionLocalHL), "OptionLocalHighsAndLows" };

				tagID.push_back(tagDBInterface[p1]);
				tagID.push_back(tagDBInterface[p2]);
				tagID.push_back(tagDBInterface[p3]);
				tagID.push_back(tagDBInterface[p4]);
				tagID.push_back(tagDBInterface[p5]);
				tagID.push_back(tagDBInterface[p6]);
				tagID.push_back(tagDBInterface[p7]);
				tagID.push_back(tagDBInterface[p8]);
				tagID.push_back(tagDBInterface[p9]);
				tagID.push_back(tagDBInterface[p10]);
				tagID.push_back(tagDBInterface[p11]);
				tagID.push_back(tagDBInterface[p12]);

				return tagID;
			}

			// Convert alert tag sequence into bitmap to compress tag combination
			uint64_t convertToBitmask(const std::vector<int>& tagID) {
				uint64_t bitmask = 0;

				for (int tagIndex : tagID) {
					if (tagIndex >= 64 || tagIndex < 0) {
						throw std::runtime_error("Invalid tag index. Must be between 0 and 63.");
					}
					bitmask |= (1ULL << tagIndex);
				}

				return bitmask;
			}

			std::vector<int> bitmaskToTags(uint64_t mask) {
				std::vector<int> tags;
				for (int i = 0; i < 64; ++i) {
					if (mask & (1ull << i)) {
						tags.push_back(i);
					}
				}
				return tags;
			}

			void initialize() {
				tagDBInterface.insert({ {Alerts::EnumString::option_type(Alerts::OptionType::Call), "OptionType"}, 1 });
				tagDBInterface.insert({ {Alerts::EnumString::option_type(Alerts::OptionType::Put), "OptionType"}, 2 });

				tagDBInterface.insert({ {time_frame(TimeFrame::FiveSecs), "TimeFrame"}, 3 });
				tagDBInterface.insert({ {time_frame(TimeFrame::ThirtySecs), "TimeFrame"}, 4 });
				tagDBInterface.insert({ {time_frame(TimeFrame::OneMin), "TimeFrame"}, 5 });
				tagDBInterface.insert({ {time_frame(TimeFrame::FiveMin), "TimeFrame"}, 6 });

				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ATM), "RelativeToMoney"}, 7 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ITM1), "RelativeToMoney"}, 8 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ITM2), "RelativeToMoney"}, 9 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ITM3), "RelativeToMoney"}, 10 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::ITM4), "RelativeToMoney"}, 11 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::DeepITM), "RelativeToMoney"}, 12 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::OTM1), "RelativeToMoney"}, 13 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::OTM2), "RelativeToMoney"}, 14 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::OTM3), "RelativeToMoney"}, 15 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::OTM4), "RelativeToMoney"}, 16 });
				tagDBInterface.insert({ {Alerts::EnumString::relative_to_money(Alerts::RelativeToMoney::DeepOTM), "RelativeToMoney"}, 17 });

				tagDBInterface.insert({ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour1), "TimeOfDay"}, 18 });
				tagDBInterface.insert({ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour2), "TimeOfDay"}, 19 });
				tagDBInterface.insert({ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour3), "TimeOfDay"}, 20 });
				tagDBInterface.insert({ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour4), "TimeOfDay"}, 21 });
				tagDBInterface.insert({ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour5), "TimeOfDay"}, 22 });
				tagDBInterface.insert({ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour6), "TimeOfDay"}, 23 });
				tagDBInterface.insert({ {Alerts::EnumString::time_of_day(Alerts::TimeOfDay::Hour7), "TimeOfDay"}, 24 });

				tagDBInterface.insert({ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::Over1), "VolumeStDev"}, 25 });
				tagDBInterface.insert({ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::Over2), "VolumeStDev"}, 26 });
				tagDBInterface.insert({ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::Over3), "VolumeStDev"}, 27 });
				tagDBInterface.insert({ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::Over4), "VolumeStDev"}, 28 });
				tagDBInterface.insert({ {Alerts::EnumString::vol_st_dev(Alerts::VolumeStDev::LowVol), "VolumeStDev"}, 29 });

				tagDBInterface.insert({ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::Vol100), "VolumeThreshold"}, 30 });
				tagDBInterface.insert({ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::Vol250), "VolumeThreshold"}, 31 });
				tagDBInterface.insert({ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::Vol500), "VolumeThreshold"}, 32 });
				tagDBInterface.insert({ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::Vol1000), "VolumeThreshold"}, 33 });
				tagDBInterface.insert({ {Alerts::EnumString::vol_threshold(Alerts::VolumeThreshold::LowVol), "VolumeThreshold"}, 34 });

				tagDBInterface.insert({ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Under1), "UnderlyingPriceDelta"}, 35 });
				tagDBInterface.insert({ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Under2), "UnderlyingPriceDelta"}, 36 });
				tagDBInterface.insert({ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Over2), "UnderlyingPriceDelta"}, 37 });

				tagDBInterface.insert({ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Under1), "OptionPriceDelta"}, 38 });
				tagDBInterface.insert({ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Under2), "OptionPriceDelta"}, 39 });
				tagDBInterface.insert({ {Alerts::EnumString::price_delta(Alerts::PriceDelta::Over2), "OptionPriceDelta"}, 40 });

				tagDBInterface.insert({ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::NDL), "UnderlyingDailyHighsAndLows"}, 41 });
				tagDBInterface.insert({ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::NDH), "UnderlyingDailyHighsAndLows"}, 42 });
				tagDBInterface.insert({ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::Inside), "UnderlyingDailyHighsAndLows"}, 43 });

				tagDBInterface.insert({ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::NDL), "OptionDailyHighsAndLows"}, 44 });
				tagDBInterface.insert({ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::NDH), "OptionDailyHighsAndLows"}, 45 });
				tagDBInterface.insert({ {Alerts::EnumString::daily_highs_and_lows(Alerts::DailyHighsAndLows::Inside), "OptionDailyHighsAndLows"}, 46 });

				tagDBInterface.insert({ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::NLL), "UnderlyingLocalHighsAndLows"}, 47 });
				tagDBInterface.insert({ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::NLH), "UnderlyingLocalHighsAndLows"}, 48 });
				tagDBInterface.insert({ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::Inside), "UnderlyingLocalHighsAndLows"}, 49 });

				tagDBInterface.insert({ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::NLL), "OptionLocalHighsAndLows"}, 50 });
				tagDBInterface.insert({ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::NLH), "OptionLocalHighsAndLows"}, 51 });
				tagDBInterface.insert({ {Alerts::EnumString::local_highs_and_lows(Alerts::LocalHighsAndLows::Inside), "OptionLocalHighsAndLows"}, 52 });
			}

		private:
			std::unordered_map<std::pair<string, string>, int, PairHash> tagDBInterface;
		};
	}
}