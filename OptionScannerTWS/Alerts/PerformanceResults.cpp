#include "PerformanceResults.h"

namespace Alerts {

	PerformanceResults::PerformanceResults(std::shared_ptr<CandleTags> candle) : ct(candle) {

		initTime = std::chrono::steady_clock::now();
	}

}