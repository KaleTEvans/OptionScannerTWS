#pragma once

#include <memory>
#include <unordered_map>

#include "SQLSchema.h"
#include "../Enums.h"

#include "../Alerts/AlertTags.h"

#include "../Enums.h"
#include "../Logger.h"

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

		inline void initializeTagTable(nanodbc::connection conn) {
			AlertTagDBInterface dbi;
			std::unordered_map<std::pair<string, string>, int, PairHash> ti = dbi.tagInterface();

			try {
				nanodbc::statement stmt(conn);

				for (auto i : ti) {
					stmt.prepare("INSERT INTO AlertTags (TagID, TagName, TagType, WeightedWins, UnweightedWins, AverageWin, Total)"
						"VALUES (?, ?, ?, ?, ?, ?, ?)");

					int TagId = i.second;
					string TagName = i.first.first;
					string TagType = i.first.second;
					const int WeightedWins = 0;
					const int UnweightedWins = 0;
					const int AverageWin = 0;
					const int Total = 0;

					stmt.bind(0, &TagId);
					stmt.bind(1, TagName.c_str());
					stmt.bind(2, TagType.c_str());
					stmt.bind(3, &WeightedWins);
					stmt.bind(4, &UnweightedWins);
					stmt.bind(5, &AverageWin);
					stmt.bind(6, &Total);

					stmt.execute();

					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void setTagTable(nanodbc::connection conn) {
			try {
				nanodbc::execute(conn, "DROP TABLE IF EXISTS AlertTags");

				string sql = "CREATE TABLE AlertTags ("
					"TagID INT PRIMARY KEY,"
					"TagName VARCHAR(20) NOT NULL,"
					"TagType VARCHAR(30) NOT NULL,"
					"WeightedWins DECIMAL(30, 3) NOT NULL,"
					"UnweightedWins DECIMAL(25, 0) NOT NULL,"
					"AverageWin DECIMAL(5, 2) NOT NULL,"
					"Total DECIMAL(20, 3) NOT NULL);";

				nanodbc::execute(conn, sql);

				initializeTagTable(conn);

				OPTIONSCANNER_DEBUG("Tag Table Initialized");
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}
	}
}