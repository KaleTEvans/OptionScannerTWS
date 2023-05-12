//=============================================================================
// OptionScanner will retrieve SPX price data every minute during market hours,
// use that price to determine the closest 12 call and put strikes, and gather
// volume data to determine if any unusual volume spikes have occured
// ============================================================================
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <ctime>
#include <windows.h>
#include <cstdlib>

#include "tWrapper.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;

class OptionScanner {

public:
	OptionScanner(tWrapper& YW, const char* host);
	~OptionScanner();

	void getDateTime();
	void retreiveSPXPrice();


private:
	tWrapper YW;
	const char* host;

	// One contract for price, one variable contract for option strikes
	Contract SPXunderlying;
	Contract SPXchain;

	vector<int> todayDate;

	vector<int> prices;
	vector<int> strikes;
	int strike;

// Variables for public use
public:
	EClientL0* EC;
};