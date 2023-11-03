#pragma once

#include <memory>

#include "SQLSchema.h"
#include "../Candle.h"
#include "../Enums.h"
//#include "../Logger.h"

using std::string;

namespace OptionDB {

	namespace CandleTables {

		// This will copy data from the candles directly for db insertion
		struct CandleForDB {
			CandleForDB(int reqId, string date, long time, double open, double high, double low, double close, long volume) :
				reqId_(reqId), date_(date), time_(time), open_(open), high_(high), low_(low), close_(close), volume_(volume) {}

			int reqId_;
			string date_;
			long time_;
			double open_;
			double high_;
			double low_;
			double close_;
			long volume_;
		};


		inline void setTable(nanodbc::connection conn) {
			try {
				nanodbc::execute(conn, "DROP TABLE IF EXISTS Candles");

				string sql = "CREATE TABLE Candles ("
					"ID INT IDENTITY(1, 1),"
					"ReqID INT NOT NULL,"
					"Date VARCHAR(10),"
					"Time INT NOT NULL,"
					"[Open] DECIMAL(16, 3) NOT NULL,"
					"[Close] DECIMAL(16, 3) NOT NULL,"
					"High DECIMAL(16, 3) NOT NULL,"
					"Low DECIMAL(16, 3) NOT NULL,"
					"Volume BIGINT,"
					"TimeFrame VARCHAR(20),"
					"CONSTRAINT Alert_Candle UNIQUE (ReqId, Time, TimeFrame));";

				nanodbc::execute(conn, sql);

				//OPTIONSCANNER_DEBUG("Candles Table initialized");
				std::cout << "Candles Table Initialized" << std::endl;
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline std::vector<Candle> get(nanodbc::connection conn, TimeFrame tf, int reqId = 0, string date = "", long time = 0) {
			std::vector<Candle> candles;
			string tfstring = time_frame(tf);

			nanodbc::statement stmt(conn);

			if (reqId == 0 && date == "" && time == 0) {
				stmt.prepare("SELECT * FROM Candles WHERE TimeFrame = ?");
				stmt.bind(0, tfstring.c_str());
			}

			try {
				nanodbc::result res = stmt.execute();

				while (res.next()) {
					TickerId reqId = res.get<TickerId>("ReqID");
					IBString date = res.get<nanodbc::string>("Date");
					long time = res.get<long>("Time");
					double open = res.get<double>("Open");
					double high = res.get<double>("High");
					double low = res.get<double>("Low");
					double close = res.get<double>("Close");
					long volume = res.get<long>("Volume");
					string timeFrame = res.get<nanodbc::string>("TimeFrame");

					Candle c(reqId, time, open, high, low, close, volume);
					candles.push_back(c);
				}
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}

			return candles;
		}

		inline void post(nanodbc::connection conn, CandleForDB& candle, TimeFrame tf) {
			try {
				nanodbc::statement stmt(conn);
				stmt.prepare("INSERT INTO Candles (ReqID, Date, Time, [Open], [Close], High, Low, Volume, TimeFrame)"
					"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

				int reqId = candle.reqId_;
				string date = candle.date_.substr(0, 8); // Just get the date, no need for time
				long time = candle.time_;
				double open = candle.open_;
				double close = candle.close_;
				double high = candle.high_;
				double low = candle.low_;
				long volume = candle.volume_;
				string timeframe = time_frame(tf);
				
				stmt.bind(0, &reqId);
				stmt.bind(1, date.c_str());
				stmt.bind(2, &time);
				stmt.bind(3, &open);
				stmt.bind(4, &close);
				stmt.bind(5, &high);
				stmt.bind(6, &low);
				stmt.bind(7, &volume);
				stmt.bind(8, timeframe.c_str());

				stmt.execute();
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void remove(nanodbc::connection conn, string& date, TimeFrame tf) {
			try {
				string tfstring = time_frame(tf);
				nanodbc::statement stmt(conn);
				stmt.prepare("DELETE FROM Candles WHERE Date = ? AND TimeFrame = ?");

				stmt.bind(0, date.c_str());
				stmt.bind(0, tfstring.c_str());
				stmt.execute();
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void remove(nanodbc::connection conn, long time, TimeFrame tf) {
			try {
				string tfstring = time_frame(tf);
				nanodbc::statement stmt(conn);
				stmt.prepare("DELETE FROM Candles WHERE Time = ? AND TimeFrame = ?");

				stmt.bind(0, &time);
				stmt.bind(1, tfstring.c_str());
				stmt.execute();
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}
	}
}