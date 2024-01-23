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

		inline long post(nanodbc::connection conn, long unixTime) {
			long maxUnixTime = -1;
			try {
				nanodbc::statement stmt(conn);
				stmt.prepare("INSERT INTO UnixValues (Time) VALUES (?)");

				stmt.bind(0, &unixTime);
				stmt.execute();

				nanodbc::statement maxTimeStmt(conn);
				maxTimeStmt.prepare("SELECT MAX(Time) FROM UnixValues;");
				auto result = maxTimeStmt.execute();
				if (result.next()) {
					maxUnixTime = result.get<long>(0);
					OPTIONSCANNER_DEBUG("Most Recent Unix Time Added: {}", maxUnixTime);
				}
			}
			catch (const std::exception& e) {
				OPTIONSCANNER_ERROR("Error: {}", e.what());
			}

			return maxUnixTime;
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

		inline void post(nanodbc::connection conn, std::vector<std::shared_ptr<CandleTags>> candle) {
			try {
				// Output table for identity key retrieval
				nanodbc::execute(conn, "DROP TABLE IF EXISTS tempId");
				nanodbc::execute(conn, "CREATE TABLE tempId (ID INT)");

				nanodbc::statement stmt(conn);

				stmt.prepare("INSERT INTO OptionCandles (ReqID, Date, Time, [Open], [Close], High, Low, Volume, TimeFrame,"
					"OptionType, TimeOfDay, RelativeToMoney, VolumeStDev, VolumeThreshold, OptPriceDelta, DailyHighLow, LocalHighLow,"
					"UnderlyingPriceDelta, UnderlyingDailyHighLow, UnderlyingLocalHighLow)"
					" OUTPUT INSERTED.ID INTO tempId"
					" VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

				size_t elements = candle.size();

				std::vector<int> reqId;
				std::vector<string> date;
				std::vector<long> time;
				std::vector<double> open;
				std::vector<double> close;
				std::vector<double> high;
				std::vector<double> low;
				std::vector<long> volume;
				std::vector<int> timeFrame;
				std::vector<int> optionType;
				std::vector<int> timeOfDay;
				std::vector<int> relativeToMoney;
				std::vector<int> volumeStDev;
				std::vector<int> volumeThreshold;
				std::vector<int> optPriceDelta;
				std::vector<int> dailyHighLow;
				std::vector<int> localHighLow;
				std::vector<int> underlyingPriceDelta;
				std::vector<int> underlyingDHL;
				std::vector<int> underlyingLHL;

				for (size_t i = 0; i < candle.size(); i++) {
					reqId.push_back(candle[i]->candle.reqId());
					date.push_back(candle[i]->candle.date());
					time.push_back(candle[i]->candle.time());
					open.push_back(candle[i]->candle.open());
					close.push_back(candle[i]->candle.close());
					high.push_back(candle[i]->candle.high());
					low.push_back(candle[i]->candle.low());
					volume.push_back(candle[i]->candle.volume());
					timeFrame.push_back(Alerts::TagDBInterface::tagToInt[{time_frame(candle[i]->getTimeFrame()), Alerts::EnumString::tag_category(Alerts::TagCategory::TimeFrame)}]);
					optionType.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::option_type(candle[i]->getOptType()), Alerts::EnumString::tag_category(Alerts::TagCategory::OptionType)}]);
					timeOfDay.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::time_of_day(candle[i]->getTOD()), Alerts::EnumString::tag_category(Alerts::TagCategory::TimeOfDay)}]);
					relativeToMoney.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::relative_to_money(candle[i]->getRTM()), Alerts::EnumString::tag_category(Alerts::TagCategory::RelativeToMoney)}]);
					volumeStDev.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::vol_st_dev(candle[i]->getVolStDev()), Alerts::EnumString::tag_category(Alerts::TagCategory::VolumeStDev)}]);
					volumeThreshold.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::vol_threshold(candle[i]->getVolThresh()), Alerts::EnumString::tag_category(Alerts::TagCategory::VolumeThreshold)}]);
					optPriceDelta.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::price_delta(candle[i]->getOptPriceDelta()), Alerts::EnumString::tag_category(Alerts::TagCategory::OptionPriceDelta)}]);
					dailyHighLow.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::daily_highs_and_lows(candle[i]->getDHL()), Alerts::EnumString::tag_category(Alerts::TagCategory::OptionDailyHighsAndLows)}]);
					localHighLow.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::local_highs_and_lows(candle[i]->getLHL()), Alerts::EnumString::tag_category(Alerts::TagCategory::OptionLocalHighsAndLows)}]);
					underlyingPriceDelta.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::price_delta(candle[i]->getUnderlyingPriceDelta()), Alerts::EnumString::tag_category(Alerts::TagCategory::UnderlyingPriceDelta)}]);
					underlyingDHL.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::daily_highs_and_lows(candle[i]->getUnderlyingDHL()), Alerts::EnumString::tag_category(Alerts::TagCategory::UnderlyingDailyHighsAndLows)}]);
					underlyingLHL.push_back(Alerts::TagDBInterface::tagToInt[{Alerts::EnumString::local_highs_and_lows(candle[i]->getUnderlyingLHL()), Alerts::EnumString::tag_category(Alerts::TagCategory::UnderlyingLocalHighsAndLows)}]);
				}

				stmt.bind(0, reqId.data(), elements);
				stmt.bind_strings(1, date);
				stmt.bind(2, time.data(), elements);
				stmt.bind(3, open.data(), elements);
				stmt.bind(4, close.data(), elements);
				stmt.bind(5, high.data(), elements);
				stmt.bind(6, low.data(), elements);
				stmt.bind(7, volume.data(), elements);
				stmt.bind(8, timeFrame.data(), elements);
				stmt.bind(9, optionType.data(), elements);
				stmt.bind(10, timeOfDay.data(), elements);
				stmt.bind(11, relativeToMoney.data(), elements);
				stmt.bind(12, volumeStDev.data(), elements);
				stmt.bind(13, volumeThreshold.data(), elements);
				stmt.bind(14, optPriceDelta.data(), elements);
				stmt.bind(15, dailyHighLow.data(), elements);
				stmt.bind(16, localHighLow.data(), elements);
				stmt.bind(17, underlyingPriceDelta.data(), elements);
				stmt.bind(18, underlyingDHL.data(), elements);
				stmt.bind(19, underlyingLHL.data(), elements);

				nanodbc::transact(stmt, elements);

				auto res = nanodbc::execute(conn, "SELECT ID FROM tempId");
				int i = 0;

				while (res.next()) {
					int lastId = res.get<int>(0);
					//std::cout << "Last Inserted ID: " << lastId << std::endl;
					candle[i]->setSqlId(lastId);
					i++;
				}
				OPTIONSCANNER_DEBUG("OptionCandle Batch Insertion Successful");
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