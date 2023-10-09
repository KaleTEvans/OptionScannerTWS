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

	void DatabaseManager::resetCandleTables() {
		CandleTables::setTable(*conn_, TimeFrame::FiveSecs);
		CandleTables::setTable(*conn_, TimeFrame::ThirtySecs);
		CandleTables::setTable(*conn_, TimeFrame::OneMin);
		CandleTables::setTable(*conn_, TimeFrame::FiveMin);
	}

	void DatabaseManager::candleInsertionLoop() {
		while (!stopInsertion) {
			std::unique_lock<std::mutex> lock(queueMtx);
			// wait for data or exit signal
			cv.wait(lock, [this]() {
				return !candleQueue.empty() || stopInsertion;
			});

			if (stopInsertion) break;

			CandleTables::post(conn_, candleQueue.front().first, candleQueue.front().second);
			candleQueue.pop();
		}
	}
}