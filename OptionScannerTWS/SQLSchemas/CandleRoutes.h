#pragma once

#include <memory>

#include "SQLSchema.h"
#include "../Candle.h"
#include "../Enums.h"

using std::string;

namespace OptionDB {

	namespace CandleTables {

		void setTable(nanodbc::connection conn, TimeFrame tf) {
			string tfstring = time_frame(tf);

			nanodbc::execute(conn, "DROP TABLE IF EXISTS" + tfstring);

			string sql = "CREATE TABLE " + tfstring + " ("
				"id INT IDENTITY(1, 1) PRIMARY KEY,"
				"reqId INT NOT NULL,"
				"date VARCHAR(10),"
				"time INT NOT NULL,"
				"[open] DECIMAL(16, 3) NOT NULL,"
				"[close] DECIMAL(16, 3) NOT NULL,"
				"high DECIMAL(16, 3) NOT NULL,"
				"low DECIMAL(16, 3) NOT NULL,"
				"volume BIGINT);";

			nanodbc::execute(conn, sql);
		}

		std::vector<Candle> get(nanodbc::connection conn, TimeFrame tf, int reqId = 0, string date = "", long time = 0) {
			std::vector<Candle> candles;
			string tfstring = time_frame(tf);

			nanodbc::statement stmt(conn);

			if (reqId == 0 && date == "" && time == 0) {
				stmt.prepare("SELECT * FROM " + tfstring);
			}

			try {
				nanodbc::result res = stmt.execute();

				while (res.next()) {
					TickerId reqId = res.get<TickerId>("reqId");
					IBString date = res.get<nanodbc::string>("date");
					long time = res.get<long>("time");
					double open = res.get<double>("open");
					double high = res.get<double>("high");
					double low = res.get<double>("low");
					double close = res.get<double>("close");
					long volume = res.get<long>("volume");

					Candle c(reqId, time, open, high, low, close, volume);
					candles.push_back(c);
				}
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}

			return candles;
		}

		void post(nanodbc::connection conn, std::shared_ptr<Candle> candle, TimeFrame tf) {
			try {
				string tfstring = time_frame(tf);

				nanodbc::statement stmt(conn);
				stmt.prepare("INSERT INTO " + tfstring + " (reqId, date, time, [open], [close], high, low, volume)"
					"VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

				int reqId = candle->reqId();
				string date = candle->date().substr(0, 8); // Just get the date, no need for time
				long time = candle->time();
				double open = candle->open();
				double close = candle->close();
				double high = candle->high();
				double low = candle->low();
				long volume = candle->volume();
				
				stmt.bind(0, &reqId);
				stmt.bind(1, date.c_str());
				stmt.bind(2, &time);
				stmt.bind(3, &open);
				stmt.bind(4, &close);
				stmt.bind(5, &high);
				stmt.bind(6, &low);
				stmt.bind(7, &volume);

				stmt.execute();
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
		}

		void remove(nanodbc::connection conn, string& date, TimeFrame tf) {
			try {
				string tfstring = time_frame(tf);
				nanodbc::statement stmt(conn);
				stmt.prepare("DELETE FROM " + tfstring + " WHERE date = ? ");

				stmt.bind(0, date.c_str());
				stmt.execute();
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
		}

		void remove(nanodbc::connection conn, long time, TimeFrame tf) {
			try {
				string tfstring = time_frame(tf);
				nanodbc::statement stmt(conn);
				stmt.prepare("DELETE FROM " + tfstring + " WHERE time = ? ");

				stmt.bind(0, &time);
				stmt.execute();
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
		}
	}
}