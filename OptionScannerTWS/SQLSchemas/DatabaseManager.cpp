#include "DatabaseManager.h"

namespace OptionDB {

	DatabaseManager::DatabaseManager() {
		conn_ = std::make_shared<nanodbc::connection>(connectToDB());
	}

	void DatabaseManager::start() {
		// Start the db insertion thread
		dbInsertionThread = std::thread([this]() {
			candleInsertionLoop();
		});
	}

	void DatabaseManager::stop() {
		// Signal insertion thread to stop
		stopInsertion = true;
		if (dbInsertionThread.joinable()) dbInsertionThread.join();
	}

	bool DatabaseManager::processingComplete() const { return processingComplete_; }

	void DatabaseManager::addToInsertionQueue(std::shared_ptr<CandleTags> ct) {
		std::unique_lock<std::mutex> lock(queueMtx);
		candlePriorityQueue.push(ct);
		lock.unlock();
		OPTIONSCANNER_DEBUG("Inserted to candle queue, current size: {}", candlePriorityQueue.size());
	}

	void DatabaseManager::addToInsertionQueue(std::shared_ptr<Candle> c, TimeFrame tf) {

		// Copy data into underlyingQueue for insertion
		UnderlyingTable::CandleForDB candle(
			c->reqId(),
			c->date(),
			c->time(),
			c->open(),
			c->high(),
			c->low(),
			c->close(),
			c->volume()
		);

		underlyingQueue.push({ candle, tf });
	}

	void DatabaseManager::addToInsertionQueue(std::shared_ptr<Alerts::PerformanceResults> pfr) {
		performanceQueue.push(pfr);
		OPTIONSCANNER_DEBUG("Inserted to performance queue, current size: {}", performanceQueue.size());
	}

	void DatabaseManager::resetCandleTables() {
		OptionDB::resetCandleTables(*conn_);
		setCandleTables();
	}

	int DatabaseManager::getUnderlyingCount() { return UnderlyingTable::candleCount(*conn_); }
	int DatabaseManager::getOptionCount() { return OptionTable::candleCount(*conn_); }

	void DatabaseManager::setCandleTables() {
		UnixTable::setTable(*conn_);
		UnderlyingTable::setTable(*conn_);
		OptionTable::setTable(*conn_);
		CandlePerformance::setTable(*conn_);
	}

	void DatabaseManager::setAlertTables() {
		AlertTables::setTagTable(*conn_);
		AlertTables::setAlertTable(*conn_);
		AlertTables::setTagMappingTable(*conn_);
		AlertTables::setAlertCombinationTable(*conn_);
	}

	void DatabaseManager::candleInsertionLoop() {
		static long lastInsertedTime = -1;

		while (true) {

			while (!underlyingQueue.empty() || !candlePriorityQueue.empty() || !performanceQueue.empty()) {
				static long prevUnixTime = -1;
				static bool unixTimeUpdated = false;

				std::vector<std::shared_ptr<CandleTags>> optionBatch;
				std::vector<std::shared_ptr<Alerts::PerformanceResults>> performanceBatch;

				if (!underlyingQueue.empty()) {
					std::unique_lock<std::mutex> lock(queueMtx);
					long unixTime = underlyingQueue.front().first.time_;
					if (unixTime != prevUnixTime) prevUnixTime = OptionDB::UnixTable::post(*conn_, unixTime);
					unixTimeUpdated = true;
					lock.unlock();

					OptionDB::UnderlyingTable::post(*conn_, underlyingQueue.front().first, underlyingQueue.front().second);
					underlyingQueue.pop();
				}

				if (!candlePriorityQueue.empty() && unixTimeUpdated) {
					std::unique_lock<std::mutex> lock(queueMtx);
					while (!candlePriorityQueue.empty() && candlePriorityQueue.top()->candle.time() <= prevUnixTime) {
						std::shared_ptr<CandleTags> ct = candlePriorityQueue.top();
						optionBatch.push_back(ct);
						candlePriorityQueue.pop();
					}
					lock.unlock();

					unixTimeUpdated = false;
				}

				if (optionBatch.size() > 0) OptionDB::OptionTable::post(*conn_, optionBatch);
				optionBatch.clear();

				if (!performanceQueue.empty()) {
					while (!performanceQueue.empty()) {
						std::shared_ptr<Alerts::PerformanceResults> pf = performanceQueue.front();
						performanceBatch.push_back(pf);
						performanceQueue.pop();
					}
				}

				if (performanceBatch.size() > 0) OptionDB::CandlePerformance::post(*conn_, performanceBatch);
				performanceBatch.clear();
			}

			// Re-check stop-insertion after processing
			if (stopInsertion) {
				std::unique_lock<std::mutex> relock(queueMtx);
				if (underlyingQueue.empty() && candlePriorityQueue.empty() && performanceQueue.empty()) break;
			}
		}
	}

	std::mutex& DatabaseManager::getMtx() { return queueMtx; }
	std::condition_variable& DatabaseManager::getCV() { return cv; }
}