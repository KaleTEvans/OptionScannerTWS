//=============================================================================
// App acts as the intermediary between the TwsApiC++ library and the 
// rest of the program
// ============================================================================
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <ctime>
#include <cstdlib>

#include "tWrapper.h"

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

	const char* host;
};