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

		//================================================
		// TABLE SETUP
		//================================================

		inline void initializeTagTable(nanodbc::connection conn) {
			AlertTagDBInterface dbi;
			std::unordered_map<std::pair<string, string>, int, PairHash> ti = dbi.tagInterface();

			try {
				nanodbc::statement stmt(conn);

				for (auto i : ti) {
					stmt.prepare("INSERT INTO AlertTags (TagID, TagName, TagType)"
						"VALUES (?, ?, ?)");

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
					"TagType VARCHAR(30) NOT NULL);";

				nanodbc::execute(conn, sql);

				initializeTagTable(conn);

				OPTIONSCANNER_DEBUG("Tag Table Initialized");
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void setAlertTable(nanodbc::connection conn) {
			try {
				nanodbc::execute(conn, "DROP TABLE IF EXISTS Alerts");

				string sql = "CREATE TABLE Alerts ("
					"AlertID INT IDENTITY(1,1) PRIMARY KEY,"
					"ReqID INT NOT NULL,"
					"TFString VARCHAR(20) NOT NULL,"
					"AlertTime INT NOT NULL,"
					"Win DECIMAL(1,1) NOT NULL,"
					"PercentWon DECIMAL(5,2) NOT NULL,"
					"CombinationBitmap BIGINT NOT NULL,"

					"FOREIGN KEY (ReqID, TFString, AlertTime) REFERENCES Candles(ReqID, Time, TimeFrame));";

				nanodbc::execute(conn, sql);

				OPTIONSCANNER_DEBUG("Alert Table Initialized");
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void setTagMappingTable(nanodbc::connection conn) {
			try {
				nanodbc::execute(conn, "DROP TABLE IF EXISTS AlertTagMapping");

				string sql = "CREATE TABLE AlertTagMapping ("
					"MapID INT IDENTITY(1,1) PRIMARY KEY,"
					"AlertID INT NOT NULL,"
					"TagID INT NOT NULL,"

					"FOREIGN KEY (AlertID) REFERENCES Alerts (AlertID),"
					"FOREIGN KEY (TagID) REFERENCES AlertTags (TagID));";

				nanodbc::execute(conn, sql);

				OPTIONSCANNER_DEBUG("Alert Tag Combination Table Initialized");
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void setAlertCombinationTable(nanodbc::connection conn) {
			nanodbc::execute(conn, "DROP TABLE IF EXISTS TagCombinations");

			string sql = "CREATE TABLE TagCombinations ("
				"CombinationID INT IDENTITY(1, 1) PRIMARY KEY,"
				"CombinationBitmap BIGINT NOT NULL UNIQUE,"
				"TotalAlerts INT NOT NULL,"
				"WeightedWins DECIMAL(10,1) NOT NULL,"
				"WinRate DECIMAL (3,2) NOT NULL,"
				"AverageWin DECIMAL (3,2) NOT NULL);";
		}

		//==========================================================
		// TABLE INSERTION
		//==========================================================

		//inline void postAlert(nanodbc::connection conn, )
	}
}