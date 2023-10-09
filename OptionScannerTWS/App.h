//=============================================================================
// App acts as the intermediary between the TwsApiC++ library and the 
// rest of the program
// ============================================================================
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <memory>

#include "tWrapper.h"
#include "DatabaseManager.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;

class App {

public:
	App(const char* host);
	~App();

public:
	EClientL0* EC;
	tWrapper YW;

	std::shared_ptr<OptionDB::DatabaseManager> dbm; // Handles all db transactions on a separate thread

	const char* host;
};