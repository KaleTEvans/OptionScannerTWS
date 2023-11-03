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

	void DatabaseManager::addToInsertionQueue(std::shared_ptr<Candle> candle, TimeFrame tf) {
		std::unique_lock<std::mutex> lock(queueMtx);
		candleQueue.push({ candle, tf });
		cv.notify_one();
	}

	void DatabaseManager::setCandleTables() {
		CandleTables::setTable(*conn_);
	}

	void DatabaseManager::setAlertTables() {
		AlertTables::setTagTable(*conn_);
		AlertTables::setAlertTable(*conn_);
		AlertTables::setTagMappingTable(*conn_);
		AlertTables::setAlertCombinationTable(*conn_);
	}

	void DatabaseManager::candleInsertionLoop() {
		std::queue<std::pair<CandleTables::CandleForDB, TimeFrame>> buffer;

		while (!stopInsertion) {
			std::unique_lock<std::mutex> lock(queueMtx);
			// wait for data or exit signal
			cv.wait(lock, [this]() {
				return !candleQueue.empty() || stopInsertion;
			});

			if (stopInsertion) break;

			// Copy data into db candle structure for insertion
			CandleTables::CandleForDB c(
				candleQueue.front().first->reqId(),
				candleQueue.front().first->date(),
				candleQueue.front().first->time(),
				candleQueue.front().first->open(),
				candleQueue.front().first->high(),
				candleQueue.front().first->low(),
				candleQueue.front().first->close(),
				candleQueue.front().first->volume()
			);

			buffer.push({ c, candleQueue.front().second });
			candleQueue.pop();

			lock.unlock();

			while (!buffer.empty()) {
				CandleTables::post(*conn_, buffer.front().first, buffer.front().second);
				buffer.pop();
			}
		}
	}
}