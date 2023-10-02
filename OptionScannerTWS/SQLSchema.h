//====================================================
// Contains all of the sql schemas for the program
// Will just add functions as they are needed
//====================================================
#pragma once

#include <iostream>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include "tWrapper.h"
#include "nanodbc/nanodbc.h"

nanodbc::connection connectToDB() {

    // Retrieve connection configuration variables
    std::string server = std::getenv("DB_SERVER_NAME");
    std::string dbName = std::getenv("DB_NAME");
    std::string username = std::getenv("DB_USERNAME");
    std::string password = std::getenv("DB_PASSWORD");

    std::string paramStr = "Driver={ODBC Driver 17 for SQL Server};"
        "Server=tcp:" + server + ".database.windows.net,1433;"
        "Database=" + dbName + ";"
        "Uid=" + username + "@" + server + ";"
        "Pwd=" + password + ";"
        "Encrypt=yes;"
        "TrustServerCertificate=no;"
        "Connection Timeout=30;";

    try {
        nanodbc::connection conn(paramStr);

        // Check if the connection is successful
        if (conn.connected()) {
            std::cout << "Connection to database established!" << std::endl;
            
            return conn;
        }
        else {
            std::cout << "Failed to establish connection!" << std::endl;
            return nanodbc::connection();
        }
    }
    catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        return nanodbc::connection();
    }
}
//
//void resetTables(nanodbc::connection conn, bool priceTable, bool OptionTable = false, bool strikesTable = false) {
//    if (priceTable) {
//        nanodbc::execute(conn, "DROP TABLE IF EXISTS SPXPrices");
//
//        std::string sql = "CREATE TABLE SPXPrices ("
//            "Id INT IDENTITY(1, 1) PRIMARY KEY,"
//            "TimeUnix INT NOT NULL,"
//            "Price DECIMAL(18, 2));";
//        
//        nanodbc::execute(conn, sql);
//    }
//}
//
//void addPriceDatatoDB(nanodbc::connection conn, std::vector<Candle> prices) {
//    try {
//        nanodbc::statement stmt(conn);
//        stmt.prepare("INSERT INTO SPXPrices (TimeUnix, Price) VALUES (?, ?)");
//
//        std::vector<int64_t> timeUnixValues;
//        std::vector<float> closePrices;
//
//        for (const auto& i : prices) {
//
//            // Convert time string to unix values
//            std::tm tmStruct = {};
//            std::istringstream ss(i.date);
//            ss >> std::get_time(&tmStruct, "%Y%m%d %H:%M:%S");
//
//            auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tmStruct));
//            auto unixTime = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
//
//            timeUnixValues.push_back(static_cast<int64_t>(unixTime));
//            closePrices.push_back(static_cast<float>(i.close));
//        }
//
//        size_t elements = prices.size();
//
//        // Bind batch parameters
//        stmt.bind(0, timeUnixValues.data(), elements);
//        stmt.bind(1, closePrices.data(), elements);
//
//        nanodbc::transact(stmt, static_cast<long>(prices.size()));
//    }
//    catch (const std::exception& ex) {
//        std::cout << "Error: " << ex.what() << std::endl;
//    }
//}