#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <chrono>
#include <ctime>

#include "Candle.h"

namespace Alerts {

	struct PerformanceResults {
		std::shared_ptr<CandleTags> ct;
		std::chrono::steady_clock::time_point initTime;

		float winLoss{ -1 };
		double winLossPct{ 0 };
		int timeToWin{ 0 };

		PerformanceResults(std::shared_ptr<CandleTags> candle);
	};

}