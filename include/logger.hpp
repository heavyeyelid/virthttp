//
// Created by hugo1 on 02.02.2019.
//
#pragma once

#include <iostream>
#include <gsl/gsl>

inline std::string config_file_loc = "config.ini";

class Logger : public std::string
{
public:
    template <typename T>
    void warning(T msg) {
        if (!isQuiet)
            std::cout << "WARN: " << msg << std::endl;
    }
  template <typename T>
    void error(T msg) {
        if (!isQuiet)
          std::cerr << "ERROR: " << msg << std::endl;
    }
    template <typename T>
    void info(T msg)
    {
        if (!isQuiet)
            std::cout << "INFO: " << msg << std::endl;
    }
    template <typename T>
    void debug(T msg)
    {
        if(isDebug){
            if(!isQuiet)
                std::cout << "DEBUG: " << msg << std::endl;
        }
    }

    void warning(const std::string& msg) {
        if (!isQuiet)
            std::cout << "WARN: " << msg << std::endl;
    }
    void error(const std::string& msg) {
        if (!isQuiet)
            std::cerr << "ERROR: " << msg << std::endl;
    }
    void info(const std::string& msg)
    {
        if (!isQuiet)
            std::cout << "INFO: " << msg << std::endl;
    }
    void debug(const std::string& msg)
    {
        if(isDebug){
            if(!isQuiet)
                std::cout << "DEBUG: " << msg << std::endl;
        }
    }

    void setQuiet(bool b) {
        isQuiet = b;
    }
    void setDebug(bool b) {
        isDebug = b;
    }

private:
    bool isQuiet = false;
    bool isDebug = false;
};

inline Logger logger;