#define _CRT_SECURE_NO_WARNINGS

#include "../pch.h"

#include "Enums.h"
#include "Alerts/AlertTags.h"

#include <map>

using namespace testing;
using namespace Alerts;

TEST(alertTagTests, tagMapFunctionality) {
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

	EXPECT_EQ(testMap.at(testAlert), 1);
}