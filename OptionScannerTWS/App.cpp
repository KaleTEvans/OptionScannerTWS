#include "App.h"

App::App(const char* host) : host(host), YW(23, false) {

    // Initialize connection
    EC = EClientL0::New(&YW);
    // Connect to TWS
    EC->eConnect(host, 7496, 0);

    // NOTE : Some API functions will encounter issues if called immediately after connection
    //          Here we will start with a simple request for the TWS current time and wait
    // ** Also note that this will return time in Unix form for the current time zone of the server hosting tws
    EC->reqCurrentTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EC->checkMessages();
    
    std::cout << YW.getCurrentTime() << std::endl;

    // Create connection to database
    dbm = std::make_shared<OptionDB::DatabaseManager>();
    dbm->start();
}

App::~App() {
    EC->eDisconnect();
    delete EC;

}