//
// Created by hugo1 on 02.02.2019.
//
#pragma once

#include <iostream>

class Logger : public std::string
{
public:
    void warning(const std::string& msg) {
        logging = msg;
        if (!isQuiet)
            std::cout << "WARN: " << msg << std::endl;
    }
    void error(const std::string& msg) {
        logging = msg;
        if (!isQuiet)
            std::cerr << "ERROR: " << msg << std::endl;
    }
    void info(const std::string& msg)
    {
        logging = msg;
        if (!isQuiet)
            std::cout << "INFO: " << msg << std::endl;
    }
    void debug(const std::string& msg)
    {
        if(isDebug){
            logging = msg;
            if(!isQuiet)
                std::cout << "DEBUG: " << msg << std::endl;
        }
    }
    const char* getLogs() {
        return logging.c_str();
    }
    void setQuiet(bool b) {
        isQuiet = b;
    }
    void setDebug(bool b) {
        isDebug = b;
    }

private:
    std::string logging;
    bool isQuiet = false;
    bool isDebug = false;
};