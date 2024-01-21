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
		candleProcessingQueue.push(ct);
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
		while (true) {
			std::unique_lock<std::mutex> lock(queueMtx);
			// wait for data or exit signal
			cv.wait(lock, [this]() {
				return !candleProcessingQueue.empty() || !underlyingQueue.empty() || stopInsertion;
				});

			// Exit if there is no more data to be processed 
			if (stopInsertion && candleProcessingQueue.empty() && underlyingQueue.empty()) break;

			lock.unlock();

			while (!candleProcessingQueue.empty() || !underlyingQueue.empty()) {

				if (!underlyingQueue.empty()) {
					long unixTime = underlyingQueue.front().first.time_;
					if (timeSet.find(unixTime) == timeSet.end()) {
						timeSet.insert(unixTime);
						UnixTable::post(*conn_, unixTime);
					}

					UnderlyingTable::post(*conn_, underlyingQueue.front().first, underlyingQueue.front().second);
					underlyingQueue.pop();
				}

				if (!candleProcessingQueue.empty()) {

					OptionTable::post(*conn_, candleProcessingQueue.front());
					candleProcessingQueue.pop();
				}
			}

			// Re-check stop-insertion after processing
			if (stopInsertion) {
				std::unique_lock<std::mutex> relock(queueMtx);
				if (candleProcessingQueue.empty() && underlyingQueue.empty()) break;
			}
		}
	}

	std::mutex& DatabaseManager::getMtx() { return queueMtx; }
	std::condition_variable& DatabaseManager::getCV() { return cv; }
}