#pragma once

#include <memory>

#include "SQLSchema.h"
#include "../Candle.h"
#include "../Enums.h"
#include "../Logger.h"

using std::string;

namespace OptionDB {

	inline void resetCandleTables(nanodbc::connection conn) {
		nanodbc::execute(conn, "DROP TABLE IF EXISTS OptionCandles");
		nanodbc::execute(conn, "DROP TABLE IF EXISTS UnderlyingCandles");
		nanodbc::execute(conn, "DROP TABLE IF EXISTS UnixValues");
	}

	namespace UnixTable {
		inline void setTable(nanodbc::connection conn) {
			try {
				nanodbc::execute(conn, "DROP TABLE IF EXISTS UnixValues");

				string sql = "CREATE TABLE UnixValues ("
					"ID INT IDENTITY(1, 1),"
					"Time INT NOT NULL,"
					"UNIQUE(Time)); ";

				nanodbc::execute(conn, sql);
				OPTIONSCANNER_DEBUG("Unix table initialized");
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void post(nanodbc::connection conn, long unixTime) {
			try {
				nanodbc::statement stmt(conn);
				stmt.prepare("INSERT INTO UnixValues (Time) VALUES (?)");

				stmt.bind(0, &unixTime);
				stmt.execute();
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline std::vector<long> get(nanodbc::connection conn) {
			std::vector<long> unixValues;

			nanodbc::statement stmt(conn);
			stmt.prepare("SELECT * FROM UnixValues");

			try {
				nanodbc::result res = stmt.execute();

				while (res.next()) {
					long time = res.get<long>("Time");
					unixValues.push_back(time);
				}
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}

			return unixValues;
		}
	}

	namespace UnderlyingTable {

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
				nanodbc::execute(conn, "DROP TABLE IF EXISTS UnderlyingCandles");

				string sql = "CREATE TABLE UnderlyingCandles ("
					"ID INT IDENTITY(1, 1),"
					"ReqID INT NOT NULL,"
					"Date VARCHAR(10),"
					"Time INT NOT NULL,"
					"[Open] DECIMAL(16, 3) NOT NULL,"
					"[Close] DECIMAL(16, 3) NOT NULL,"
					"High DECIMAL(16, 3) NOT NULL,"
					"Low DECIMAL(16, 3) NOT NULL,"
					"Volume INT,"
					"TimeFrame VARCHAR(20) NOT NULL,"

					"FOREIGN KEY (Time) REFERENCES UnixValues(Time));";

				nanodbc::execute(conn, sql);
				OPTIONSCANNER_DEBUG("UnderlyingCandles table initialized");
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void post(nanodbc::connection conn, CandleForDB& candle, TimeFrame tf) {
			try {
				nanodbc::statement stmt(conn);
				stmt.prepare("INSERT INTO UnderlyingCandles (ReqID, Date, Time, [Open], [Close], High, Low, Volume, TimeFrame)"
					"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

				int reqId = candle.reqId_;
				string date = candle.date_.substr(0, 8); // Just get the date, no need for time
				long time = candle.time_;
				double open = candle.open_;
				double close = candle.close_;
				double high = candle.high_;
				double low = candle.low_;
				long volume = candle.volume_;
				std::string timeframe = time_frame(tf);

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

		inline std::vector<Candle> get(nanodbc::connection conn, TimeFrame tf) {
			std::vector<Candle> candles;
			string tfstring = time_frame(tf);

			nanodbc::statement stmt(conn);

			stmt.prepare("SELECT * FROM UnderlyingCandles WHERE TimeFrame = ?");
			stmt.bind(0, tfstring.c_str());

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

		inline int candleCount(nanodbc::connection conn) {
			nanodbc::statement stmt(conn);

			stmt.prepare("SELECT COUNT(*) FROM UnderlyingCandles");
			nanodbc::result res = stmt.execute();
			res.next();
			return res.get<int>(0);
		}
	}

	namespace OptionTable {

		inline void setTable(nanodbc::connection conn) {
			try {
				nanodbc::execute(conn, "DROP TABLE IF EXISTS OptionCandles");

				string sql = "CREATE TABLE OptionCandles ("
					"ID INT IDENTITY(1, 1),"
					"ReqID INT NOT NULL,"
					"Date VARCHAR(20),"
					"Time INT NOT NULL,"
					"[Open] DECIMAL(16, 3) NOT NULL,"
					"[Close] DECIMAL(16, 3) NOT NULL,"
					"High DECIMAL(16, 3) NOT NULL,"
					"Low DECIMAL(16, 3) NOT NULL,"
					"Volume INT NOT NULL,"
					"TimeFrame INT NOT NULL,"
					"OptionType INT NOT NULL,"
					"TimeOfDay INT NOT NULL,"
					"RelativeToMoney INT NOT NULL,"
					"VolumeStDev INT NOT NULL,"
					"VolumeThreshold INT NOT NULL,"
					"OptPriceDelta INT NOT NULL,"
					"DailyHighLow INT NOT NULL,"
					"LocalHighLow INT NOT NULL,"
					"UnderlyingPriceDelta INT NOT NULL,"
					"UnderlyingDailyHighLow INT NOT NULL,"
					"UnderlyingLocalHighLow INT NOT NULL,"

					"FOREIGN KEY (Time) REFERENCES UnixValues(Time),"
					"FOREIGN KEY (TimeFrame) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (OptionType) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (TimeOfDay) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (RelativeToMoney) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (VolumeStDev) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (VolumeThreshold) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (OptPriceDelta) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (DailyHighLow) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (LocalHighLow) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (UnderlyingPriceDelta) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (UnderlyingDailyHighLow) REFERENCES AlertTags(TagID),"
					"FOREIGN KEY (UnderlyingLocalHighLow) REFERENCES AlertTags(TagID));";

				nanodbc::execute(conn, sql);

				OPTIONSCANNER_DEBUG("OptionCandles Table initialized");
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline void post(nanodbc::connection conn, std::shared_ptr<CandleTags> candle) {
			try {
				nanodbc::statement stmt(conn);
				stmt.prepare("INSERT INTO OptionCandles (ReqID, Date, Time, [Open], [Close], High, Low, Volume, TimeFrame,"
					"OptionType, TimeOfDay, RelativeToMoney, VolumeStDev, VolumeThreshold, OptPriceDelta, DailyHighLow, LocalHighLow,"
					"UnderlyingPriceDelta, UnderlyingDailyHighLow, UnderlyingLocalHighLow)"
					"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

				int reqId = candle->candle.reqId();
				string date = candle->candle.date();
				long time = candle->candle.time();
				double open = candle->candle.open();
				double close = candle->candle.close();
				double high = candle->candle.high();
				double low = candle->candle.low();
				long volume = candle->candle.volume();
				int timeFrame = Alerts::TagDBInterface::tagToInt[{time_frame(candle->getTimeFrame()), Alerts::EnumString::tag_category(Alerts::TagCategory::TimeFrame)}];
				int optionType = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::option_type(candle->getOptType()), Alerts::EnumString::tag_category(Alerts::TagCategory::OptionType)}];
				int timeOfDay = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::time_of_day(candle->getTOD()), Alerts::EnumString::tag_category(Alerts::TagCategory::TimeOfDay)}];
				int relativeToMoney = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::relative_to_money(candle->getRTM()), Alerts::EnumString::tag_category(Alerts::TagCategory::RelativeToMoney)}];
				int volumeStDev = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::vol_st_dev(candle->getVolStDev()), Alerts::EnumString::tag_category(Alerts::TagCategory::VolumeStDev)}];
				int volumeThreshold = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::vol_threshold(candle->getVolThresh()), Alerts::EnumString::tag_category(Alerts::TagCategory::VolumeThreshold)}];
				int optPriceDelta = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::price_delta(candle->getOptPriceDelta()), Alerts::EnumString::tag_category(Alerts::TagCategory::OptionPriceDelta)}];
				int dailyHighLow = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::daily_highs_and_lows(candle->getDHL()), Alerts::EnumString::tag_category(Alerts::TagCategory::OptionDailyHighsAndLows)}];
				int localHighLow = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::local_highs_and_lows(candle->getLHL()), Alerts::EnumString::tag_category(Alerts::TagCategory::OptionLocalHighsAndLows)}];
				int underlyingPriceDelta = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::price_delta(candle->getUnderlyingPriceDelta()), Alerts::EnumString::tag_category(Alerts::TagCategory::UnderlyingPriceDelta)}];
				int underlyingDHL = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::daily_highs_and_lows(candle->getUnderlyingDHL()), Alerts::EnumString::tag_category(Alerts::TagCategory::UnderlyingDailyHighsAndLows)}];
				int underlyingLHL = Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::local_highs_and_lows(candle->getUnderlyingLHL()), Alerts::EnumString::tag_category(Alerts::TagCategory::UnderlyingLocalHighsAndLows)}];

				stmt.bind(0, &reqId);
				stmt.bind(1, date.c_str());
				stmt.bind(2, &time);
				stmt.bind(3, &open);
				stmt.bind(4, &close);
				stmt.bind(5, &high);
				stmt.bind(6, &low);
				stmt.bind(7, &volume);
				stmt.bind(8, &timeFrame);
				stmt.bind(9, &optionType);
				stmt.bind(10, &timeOfDay);
				stmt.bind(11, &relativeToMoney);
				stmt.bind(12, &volumeStDev);
				stmt.bind(13, &volumeThreshold);
				stmt.bind(14, &optPriceDelta);
				stmt.bind(15, &dailyHighLow);
				stmt.bind(16, &localHighLow);
				stmt.bind(17, &underlyingPriceDelta);
				stmt.bind(18, &underlyingDHL);
				stmt.bind(19, &underlyingLHL);

				stmt.execute();
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}
		}

		inline std::vector<CandleTags> get(nanodbc::connection conn) {
			std::vector<CandleTags> candles;

			nanodbc::statement stmt(conn);

			stmt.prepare("SELECT * FROM OptionCandles");

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
					int timeFrame = res.get<int>("TimeFrame");
					int optionType = res.get<int>("OptionType");
					int timeOfDay = res.get<int>("TimeOfDay");
					int relativeToMoney = res.get<int>("RelativeToMoney");
					int volumeStDev = res.get<int>("VolumeStDev");
					int volumeThreshold = res.get<int>("VolumeThreshold");
					int optPriceDelta = res.get<int>("OptPriceDelta");
					int dailyHighLow = res.get<int>("DailyHighLow");
					int localHighLow = res.get<int>("LocalHighLow");
					int underlyingPriceDelta = res.get<int>("UnderlyingPriceDelta");
					int underlyingDHL = res.get<int>("UnderlyingDailyHighLow");
					int underlyingLHL = res.get<int>("UnderlyingLocalHighLow");

					std::vector<int> tags = { timeFrame, optionType, timeOfDay, relativeToMoney, volumeStDev,
						volumeThreshold, optPriceDelta, dailyHighLow, localHighLow, underlyingPriceDelta,
						underlyingDHL, underlyingLHL };
					std::shared_ptr<Candle> c = std::make_shared<Candle>(reqId, time, open, high, low, close, volume);
					CandleTags ct(c, tags);
					candles.push_back(ct);
				}
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}

			return candles;
		}

		inline int candleCount(nanodbc::connection conn) {
			nanodbc::statement stmt(conn);

			stmt.prepare("SELECT COUNT(*) FROM OptionCandles");
			nanodbc::result res = stmt.execute();
			res.next();
			return res.get<int>(0);
		}
	}
}