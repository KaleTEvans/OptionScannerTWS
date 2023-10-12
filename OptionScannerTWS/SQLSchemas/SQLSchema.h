//====================================================
// Contains all of the sql schemas for the program
// Will just add functions as they are needed
//====================================================
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <unordered_map>

#include "nanodbc/nanodbc.h"
#include "../Logger.h"

using std::string;

namespace OptionDB {

    inline nanodbc::connection connectToDB() {

        // Retrieve connection configuration variables
        std::string server = std::getenv("DB_SERVER_NAME");
        std::string dbName = std::getenv("DB_NAME");
        std::string username = std::getenv("DB_USERNAME");
        std::string password = std::getenv("DB_PASSWORD");

        std::cout << "Server: " << std::getenv("DB_SERVER_NAME") << std::endl;

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
                //std::cout << "Connection to database established!" << std::endl;
                OPTIONSCANNER_DEBUG("Connection to database established!");

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
}