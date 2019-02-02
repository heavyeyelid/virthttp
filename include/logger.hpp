//
// Created by hugo1 on 02.02.2019.
//
#pragma once

#include <iostream>
#include <gsl/gsl>
#include "fwd.hpp"

constexpr auto config_file_loc = "config.ini";

class Logger
{
public:
    template <typename... Ts>
    void raw(std::ostream& os, Ts... msg){
      (std::cout << ... << msg);
      std::cout << std::endl;
    }

    template <typename... Ts>
    void warning(Ts... msg) {
        if (!isQuiet)
          raw(std::cout, "WARN: ", msg...);
    }
    template <typename... Ts>
    void error(Ts... msg) {
        if (!isQuiet)
          raw(std::cerr, "ERROR: ", msg...);
    }
    template <typename... Ts>
    void info(Ts... msg)
    {
        if (!isQuiet)
          raw(std::cout, "INFO: ", msg...);
    }
    template <typename... Ts>
    void debug(Ts... msg)
    {
        if(isDebug && !isQuiet)
          raw(std::cout, "DEBUG: ", msg...);
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

inline Logger logger{}; // default ctor and dtor doesn't generate any code out of main