#include "AlertHandler.h"

namespace Alerts {

	//===================================================
	// Alert Handler
	//===================================================

	AlertHandler::AlertHandler(std::shared_ptr<std::unordered_map<int, std::shared_ptr<ContractData>>> contractMap,
		std::shared_ptr<OptionDB::DatabaseManager> dbm) :
		contractMap_(contractMap), dbm_(dbm) {

		// Start a thread to check the alerts
		alertCheckThread_ = std::thread(&AlertHandler::checkAlertOutcomes, this);
		OPTIONSCANNER_DEBUG("Alert Handler Initialized");
	}

	AlertHandler::~AlertHandler() {
		doneCheckingAlerts();
	}

	void AlertHandler::inputAlert(std::shared_ptr<CandleTags> candle) {
		std::unique_lock<std::mutex> lock(alertMtx_);
		std::shared_ptr<PerformanceResults> alert = std::make_shared<PerformanceResults>(candle);
		alertUpdateQueue.push(alert);
		lock.unlock();
	}

	void AlertHandler::checkAlertOutcomes() {
		// Start time to reference and log win rate data every 30 minutes
		std::chrono::steady_clock::time_point refTime = std::chrono::steady_clock::now();

		while (!doneCheckingAlerts_) {
			while (!alertUpdateQueue.empty()) {
				// Check current time to see if 30 minutes have passed since the first alert was added to the queue
				// When queue first starts, candles will get in deadlock with the database manager
				std::unique_lock<std::mutex> lock(alertMtx_);
				std::chrono::steady_clock::time_point prevAlertTIme = alertUpdateQueue.front()->initTime;
				lock.unlock();
				std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
				std::chrono::minutes elsapsedTime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - prevAlertTIme);

				if (elsapsedTime >= std::chrono::minutes(10)) {
					std::unique_lock<std::mutex> lock(alertMtx_);

					std::shared_ptr<PerformanceResults> a = alertUpdateQueue.front();
					// Access data from the contract map
					std::vector<std::shared_ptr<Candle>> prevCandles = contractMap_->at(a->ct->candle.reqId())->candlesLast30Minutes();

					// just use the lock to access the contract map
					lock.unlock();

					checkWinStats(prevCandles, a);

					// Send to DB queue
					dbm_->addToInsertionQueue(a);

					alertUpdateQueue.pop();
				}
			}
		}
	}

	void AlertHandler::doneCheckingAlerts() {
		doneCheckingAlerts_ = true;
		alertCheckThread_.join();
	}

	//========================================================
	// Helper Functions
	//========================================================

	void checkWinStats(std::vector<std::shared_ptr<Candle>> prevCandles, std::shared_ptr<PerformanceResults> a) {
		// Ensure we start at a vector candle that is after the alert time
		// If we are close to the market close, it won't be a full 30 minutes
		size_t i = 0;

		while (a->ct->candle.time() > prevCandles[i]->time()) i++;

		double startPrice = a->ct->candle.close();
		double maxPrice = startPrice;
		double minPrice = startPrice;
		double percentChangeHigh = 0;
		double percentChangeLow = 0;
		long startTime = a->ct->candle.time();
		long timeElapsed = 0;

		for (i; i < prevCandles.size(); i++) {

			// If percentChangeLow gets to -30% break and compare to percentChangeHigh
			if (percentChangeLow <= -30.0) {
				break;
			}

			minPrice = min(minPrice, prevCandles[i]->low());
			if (maxPrice < prevCandles[i]->high()) {
				maxPrice = max(maxPrice, prevCandles[i]->high());
				timeElapsed = prevCandles[i]->time();
			}

			percentChangeLow = ((minPrice - startPrice) / startPrice) * 100;
			percentChangeHigh = ((maxPrice - startPrice) / startPrice) * 100;
		}

		if (percentChangeHigh <= 0) {
			a->winLoss = 0;
			a->winLossPct = percentChangeLow;
		}
		else  if (percentChangeHigh > 0 && percentChangeHigh < 60.0) {
			a->winLoss = 0.5;
			a->winLossPct = percentChangeHigh;
			a->timeToWin = timeElapsed - startTime;
		}
		else {
			a->winLoss = 1;
			a->winLossPct = percentChangeHigh;
			a->timeToWin = timeElapsed - startTime;
		}
	}
}