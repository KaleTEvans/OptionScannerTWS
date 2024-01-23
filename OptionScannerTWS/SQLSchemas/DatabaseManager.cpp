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
		candlePriorityQueue.push(ct);
		OPTIONSCANNER_DEBUG("Inserted to queue, current size: {}", candlePriorityQueue.size());
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

			while (!underlyingQueue.empty() || !candlePriorityQueue.empty()) {
				static long prevUnixTime = -1;
				static bool unixTimeUpdated = false;

				std::vector<std::shared_ptr<CandleTags>> optionBatch;

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
					while (!candlePriorityQueue.empty() && candlePriorityQueue.top()->candle.time() <= prevUnixTime) {
						std::shared_ptr<CandleTags> ct = candlePriorityQueue.top();
						optionBatch.push_back(ct);
						candlePriorityQueue.pop();
					}

					unixTimeUpdated = false;
				}

				if (optionBatch.size() > 0) OptionDB::OptionTable::post(*conn_, optionBatch);
				optionBatch.clear();
			}

			// Re-check stop-insertion after processing
			if (stopInsertion) {
				std::unique_lock<std::mutex> relock(queueMtx);
				if (underlyingQueue.empty() && candlePriorityQueue.empty()) break;
			}
		}
	}

	std::mutex& DatabaseManager::getMtx() { return queueMtx; }
	std::condition_variable& DatabaseManager::getCV() { return cv; }
}