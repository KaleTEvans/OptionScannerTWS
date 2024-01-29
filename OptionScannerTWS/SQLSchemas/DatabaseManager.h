#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include "SQLSchema.h"
#include "CandleRoutes.h"
#include "AlertRoutes.h"

#include <memory>
#include <queue>
#include <unordered_set>

// Comparator for OptionCandle min heap 
struct candleTimeComparator {
	int operator() (const std::shared_ptr<CandleTags> ct1, const std::shared_ptr<CandleTags> ct2) {
		return ct1->candle.time() > ct2->candle.time();
	}
};


namespace OptionDB {

	class DatabaseManager {
	public:
		DatabaseManager();

		void start();
		void stop();
		bool processingComplete() const;

		void addToInsertionQueue(std::shared_ptr<CandleTags> ct);
		void addToInsertionQueue(std::shared_ptr<Candle> c, TimeFrame tf);
		void addToInsertionQueue(std::shared_ptr<Alerts::PerformanceResults> pfr);

		void resetCandleTables();

		int getUnderlyingCount();
		int getOptionCount();

		void setCandleTables();
		void setAlertTables();

		std::mutex& getMtx();
		std::condition_variable& getCV();

	private:
		void candleInsertionLoop();

		std::shared_ptr<nanodbc::connection> conn_;

		std::thread dbInsertionThread;
		std::mutex queueMtx;
		std::condition_variable cv;
		bool stopInsertion{ false };
		bool processingComplete_{ false };

		// Processing containers
		std::queue<std::pair<UnderlyingTable::CandleForDB, TimeFrame>> underlyingQueue;
		std::priority_queue<std::shared_ptr<CandleTags>, std::vector<std::shared_ptr<CandleTags>>, candleTimeComparator> candlePriorityQueue;
		std::queue<std::shared_ptr<Alerts::PerformanceResults>> performanceQueue;

		// Keeps track of each time increment
		std::unordered_set<long> timeSet;
	};
}