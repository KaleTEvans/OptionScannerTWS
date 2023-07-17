#pragma once

#include <iostream>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <cstdlib>

#include "nanodbc/nanodbc.h"

void connectToDB() {

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
            std::cout << "Connection established!" << std::endl;
            // Perform database operations using the connection
            // ...
        }
        else {
            std::cout << "Failed to establish connection!" << std::endl;
        }
    }
    catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
}