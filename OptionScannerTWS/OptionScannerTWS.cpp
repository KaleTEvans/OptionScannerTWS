#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <ctime>

#include "tWrapper.h"
#include "App.h"


//========================================================================
// main entry
//========================================================================
int main(void) {
    ///Easier: just allocate your wrapper and instantiate the EClientL0 with it.
    const char* host = "127.0.0.1";

    OptionScanner* opt = new OptionScanner(host);

    opt->retreiveSPXPrice();
    opt->viewSPXPrice();

    //Easier: Call checkMessages() in a loop. No need to wait between two calls.
    while (opt->YW.notDone()) {
        opt->EC->checkMessages();
        
        /*if (opt->YW.Req == 101) {
            for (auto i : opt->YW.closePrices) std::cout << i << std::endl;
        }*/
        
    }

    delete opt;

    return 0;
}