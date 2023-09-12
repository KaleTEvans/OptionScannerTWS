#define _CRT_SECURE_NO_WARNINGS

#include "../pch.h"

#include "Enums.h"
#include "Alerts/AlertTags.h"

#include <map>

using namespace testing;
using namespace Alerts;

std::vector<std::pair<AlertTags, std::pair<double, double>>> createData();

TEST(alertTagTests, alertTags) {
	// Create some alerts
	AlertTags alertOne(OptionType::Call, TimeFrame::FiveMin, RelativeToMoney::ATM, TimeOfDay::Hour1, VolumeStDev::LowVol,
		VolumeThreshold::Vol100, PriceDelta::Over2, PriceDelta::Under1, DailyHighsAndLows::Inside, LocalHighsAndLows::NLH,
		DailyHighsAndLows::NDH, LocalHighsAndLows::NLH);

	AlertTags alertTwo(OptionType::Put, TimeFrame::FiveMin, RelativeToMoney::ATM, TimeOfDay::Hour1, VolumeStDev::LowVol,
		VolumeThreshold::Vol100, PriceDelta::Over2, PriceDelta::Under1, DailyHighsAndLows::Inside, LocalHighsAndLows::NLH,
		DailyHighsAndLows::NDH, LocalHighsAndLows::NLH);

	int valOne = 1;
	int valTwo = 2;

	std::map<AlertTags, int> testMap;

	testMap.insert({ alertOne, valOne });
	testMap.insert({ alertTwo, valTwo });

	AlertTags testAlert(OptionType::Call, TimeFrame::FiveMin, RelativeToMoney::ATM, TimeOfDay::Hour1, VolumeStDev::LowVol,
		VolumeThreshold::Vol100, PriceDelta::Over2, PriceDelta::Under1, DailyHighsAndLows::Inside, LocalHighsAndLows::NLH,
		DailyHighsAndLows::NDH, LocalHighsAndLows::NLH);

	AlertTags testAlertTwo(OptionType::Put, TimeFrame::FiveMin, RelativeToMoney::ATM, TimeOfDay::Hour1, VolumeStDev::LowVol,
		VolumeThreshold::Vol100, PriceDelta::Over2, PriceDelta::Under1, DailyHighsAndLows::Inside, LocalHighsAndLows::NLH,
		DailyHighsAndLows::NDH, LocalHighsAndLows::NLH);

	EXPECT_EQ(testMap.at(testAlert), 1);
	EXPECT_EQ(testMap.at(testAlertTwo), 2);
}

TEST(alertTagTests, alertStats) {
	// Use helper function to generate array of alert tag items, see below
	std::vector<std::pair<AlertTags, std::pair<double, double>>> sampleData = createData();

	AlertTagStats sampleStats;

	for (auto i : sampleData) {
		sampleStats.updateStats(i.first, i.second.first, i.second.second);
	}

	// First test that all the alerts have been added
	EXPECT_EQ(sampleStats.alertSpecificStats(sampleData[0].first).totalAlerts(), 5);
	EXPECT_EQ(sampleStats.alertSpecificStats(sampleData[5].first).totalAlerts(), 3);
	EXPECT_EQ(sampleStats.alertSpecificStats(sampleData[8].first).totalAlerts(), 5);
	EXPECT_EQ(sampleStats.alertSpecificStats(sampleData[13].first).totalAlerts(), 2);

	// Check the average win
	EXPECT_EQ(sampleStats.alertSpecificStats(sampleData[0].first).averageWin(), 80);
	EXPECT_EQ(sampleStats.alertSpecificStats(sampleData[5].first).averageWin(), 60);
	EXPECT_EQ(sampleStats.alertSpecificStats(sampleData[8].first).averageWin(), 75);
	EXPECT_EQ(sampleStats.alertSpecificStats(sampleData[13].first).averageWin(), 0);

	// Check win rates
	EXPECT_EQ(sampleStats.timeFrameStats(sampleData[0].first.timeFrame).winRate(), 0.625);
	EXPECT_EQ(sampleStats.timeOfDayStats(sampleData[0].first.timeOfDay).winRate(), 8.0/13.0);
}

std::vector<std::pair<AlertTags, std::pair<double, double>>> createData() {
	std::vector<std::pair<AlertTags, std::pair<double, double>>> sampleData;

	AlertTags alertOne(OptionType::Call, TimeFrame::FiveSecs, RelativeToMoney::ITM1, TimeOfDay::Hour1, VolumeStDev::Over3,
		VolumeThreshold::Vol1000, PriceDelta::Over2, PriceDelta::Under1, DailyHighsAndLows::Inside, LocalHighsAndLows::NLH,
		DailyHighsAndLows::NDH, LocalHighsAndLows::NLH);

	std::vector<std::pair<double, double>> winStatsOne = { {1, 100}, {0.5, 50}, {1, 100}, {0.5, 50}, {1, 100} }; // Avg win = 80%

	for (auto i = 0; i < 5; i++) sampleData.push_back({ alertOne, winStatsOne[i] });

	AlertTags alertTwo(OptionType::Put, TimeFrame::FiveSecs, RelativeToMoney::ITM1, TimeOfDay::Hour1, VolumeStDev::Over3,
		VolumeThreshold::Vol1000, PriceDelta::Over2, PriceDelta::Under1, DailyHighsAndLows::Inside, LocalHighsAndLows::NLL,
		DailyHighsAndLows::Inside, LocalHighsAndLows::NLH);

	std::vector<std::pair<double, double>> winStatsTwo = { {0, 0}, {0, 0}, {1, 60} }; // Avg win = 60%, winRate = 33%

	for (auto i = 0; i < 3; i++) sampleData.push_back({ alertTwo, winStatsTwo[i] });

	AlertTags alertThree(OptionType::Call, TimeFrame::OneMin, RelativeToMoney::OTM4, TimeOfDay::Hour1, VolumeStDev::Over3,
		VolumeThreshold::Vol1000, PriceDelta::Over2, PriceDelta::Under1, DailyHighsAndLows::Inside, LocalHighsAndLows::NLH,
		DailyHighsAndLows::NDL, LocalHighsAndLows::NLL);

	std::vector<std::pair<double, double>> winStatsThree = { {0, 0}, {1, 100}, {0.5, 50}, {1, 100}, {0.5, 50} }; // Avg win = 75%, winRate = 66%

	for (auto i = 0; i < 5; i++) sampleData.push_back({ alertThree, winStatsThree[i] });

	AlertTags alertFour(OptionType::Put, TimeFrame::OneMin, RelativeToMoney::OTM4, TimeOfDay::Hour6, VolumeStDev::Over3,
		VolumeThreshold::Vol1000, PriceDelta::Under1, PriceDelta::Under1, DailyHighsAndLows::Inside, LocalHighsAndLows::NLH,
		DailyHighsAndLows::NDL, LocalHighsAndLows::NLL);

	std::vector<std::pair<double, double>> winStatsFour = { {0, 0}, {0, 0} };

	for (auto i = 0; i < 2; i++) sampleData.push_back({ alertFour, winStatsFour[i] });

	return sampleData;
}