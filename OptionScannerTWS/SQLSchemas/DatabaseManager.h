#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include "SQLSchema.h"
#include "CandleRoutes.h"
#include "AlertRoutes.h"

#include <memory>
#include <queue>

namespace OptionDB {

	class DatabaseManager {
	public:
		DatabaseManager();

		void start();
		void stop();

		void addToInsertionQueue(std::shared_ptr<Candle> candle, TimeFrame tf);

		void setCandleTables();
		void setAlertTables();

	private:
		void candleInsertionLoop();

		std::shared_ptr<nanodbc::connection> conn_;

		std::thread dbInsertionThread;
		std::queue<std::pair<std::shared_ptr<Candle>, TimeFrame>> candleQueue;
		std::mutex queueMtx;
		std::condition_variable cv;
		bool stopInsertion{ false };
	};
}