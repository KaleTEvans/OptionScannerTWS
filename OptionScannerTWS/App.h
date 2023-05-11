//===================================================
// Connect to TWS Client, include all tws api headers
// ==================================================

#pragma once

#include <iostream>

///Easier: Just one include statement for all functionality
#include "TwsApiL0.h"

///Faster: Check spelling of parameter at compile time instead of runtime.
#include "TwsApiDefs.h"

class OptionScanner {

private:
	EClientL0* EC;

public:
	OptionScanner(EClientL0* EC) : EC(EC) {
		EC->eConnect("127.0.0.1", 7496, 0);
		std::cout << "Connected to TWS Server" << std::endl;
		EC->reqCurrentTime();
	}

	~OptionScanner() {
		EC->eDisconnect();
	}
};