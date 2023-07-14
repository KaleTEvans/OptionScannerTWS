//=============================================================================
// Functionality of the App class is to make customized data requests to the 
// TWS client, as well as return some of the formatted data from the wrapper
// ============================================================================
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <ctime>
#include <windows.h>
#include <cstdlib>
#include <unordered_set>

#include "tWrapper.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::unordered_set;

class App {

public:
	App(const char* host);
	~App();

	void createUnderlyingContract(IBString ticker, bool isIndex = true);
	void getDateTime();
	void retreiveRecentData(string interval, string duration, TickerId reqId);
	void populateStrikes(int multiple = 5, int reqId = 101);

	// Accessors
	vector<int> getDateVector() const { return todayDate; }
	Contract getUnderlyingContract() const { return underlying; }

private:
	Contract underlying;

	vector<int> todayDate;

// Variables for public use
public:
	EClientL0* EC;
	tWrapper YW;

	const char* host;
	vector<Candle> prices;
	vector<int> strikes;

	// Use test specific data
	bool useTestData = false;
};