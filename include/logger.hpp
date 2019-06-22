//
// Created by hugo1 on 02.02.2019.
//
#pragma once

#include <iostream>
#include <gsl/gsl>

class Logger {
  public:
    template <typename... Ts> void raw(std::ostream& os, Ts... msg) {
        (std::cout << ... << msg);
        std::cout << std::endl;
    }

    template <typename... Ts> void warning(Ts... msg) {
        if (!isQuiet) {
            if (isColored)
                raw(std::cout, "\033[0;33mWARN: ", msg..., "\033[0m");
            else
                raw(std::cout, "WARN: ", msg...);
        }
    }

    template <typename... Ts> void error(Ts... msg) {
        if (!isQuiet) {
            if (isColored)
                raw(std::cerr, "\033[0;31mERROR: ", msg..., "\033[0m");
            else
                raw(std::cerr, "ERROR: ", msg...);
        }
    }

    template <typename... Ts> void info(Ts... msg) {
        if (!isQuiet)
            raw(std::cout, "INFO: ", msg...);
    }

    template <typename... Ts> void debug(Ts... msg) {
        if (isDebug && !isQuiet) {
            if (isColored)
                raw(std::cout, "\033[0;32mDEBUG: ", msg..., "\033[0m");
            else
                raw(std::cout, "DEBUG: ", msg...);
        }
    }

    void setQuiet(bool b) { isQuiet = b; }

    void setDebug(bool b) { isDebug = b; }

    void setColored(bool b) { isColored = b; }

  private:
    bool isQuiet = false;
    bool isDebug = false;
    bool isColored = true;
};

inline Logger logger{}; // default ctor and dtor doesn't generate any code out of main