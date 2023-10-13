#pragma once

#include <memory>
#include <unordered_map>

#include "SQLSchema.h"
#include "AlertTagDBInterface.h"
#include "../Enums.h"

#include "../Alerts/AlertTags.h"

#include "../Enums.h"
#include "../Logger.h"

using std::string;

namespace OptionDB {
	
	namespace AlertTables {

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

		inline void setTagCombinationTable(nanodbc::connection conn) {
			try {
				nanodbc::execute(conn, "DROP TABLE IF EXISTS TagCombinationResults");

				string sql = "CREATE TABLE TagCombinationResults ("
					"id INT IDENTITY(1,1) PRIMARY KEY,"
					"AlertTime INT NOT NULL,"
					"OptionType INT NOT NULL,"
					"TimeFrame INT NOT NULL,"
					"RelativeToMoney INT NOT NULL,"
					"TimeOfDay INT NOT NULL,"
					"VolumeStDev INT NOT NULL,"
					"VolumeThreshold INT NOT NULL,"
					"UnderlyingPriceDelta INT NOT NULL,"
					"OptionPriceDelta INT NOT NULL,"
					"UnderlyingDailyHL INT NOT NULL,"
					"UnderlyingOptionHL INT NOT NULL,"
					"LocalOptionHL INT NOT NULL,"
					"LocalOptionHL INT NOT NULL,"
					"WeightedWins DECIMAL(30, 3) NOT NULL,"
					"UnweightedWins DECIMAL(25, 0) NOT NULL,"
					"AverageWin DECIMAL(5, 2) NOT NULL,"
					"Total DECIMAL(20, 3) NOT NULL);";

				nanodbc::execute(conn, sql);

				OPTIONSCANNER_DEBUG("Alert Tag Combination Table Initialized");
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void setAlertTable(nanodbc::connection conn) {
			try {
				nanodbc::execute(conn, "DROP TABLE IF EXISTS Alerts");

				string sql = "CREATE TABLE Alerts ("
					"id INT IDENTITY(1,1) PRIMARY KEY,"
					"reqId INT NOT NULL,"
					"AlertTime INT NOT NULL,"
					"OptionType INT NOT NULL,"
					"TimeFrame INT NOT NULL,"
					"RelativeToMoney INT NOT NULL,"
					"TimeOfDay INT NOT NULL,"
					"VolumeStDev INT NOT NULL,"
					"VolumeThreshold INT NOT NULL,"
					"UnderlyingPriceDelta INT NOT NULL,"
					"OptionPriceDelta INT NOT NULL,"
					"UnderlyingDailyHL INT NOT NULL,"
					"UnderlyingOptionHL INT NOT NULL,"
					"LocalOptionHL INT NOT NULL,"
					"LocalOptionHL INT NOT NULL,"
					"Win INT NOT NULL,"
					"PercentWon DECIMAL(5,2) NOT NULL);";

				nanodbc::execute(conn, sql);

				OPTIONSCANNER_DEBUG("Alert Table Initialized");
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		//inline void postAlert(nanodbc::connection conn, )
	}
}