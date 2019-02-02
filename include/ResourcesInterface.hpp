//
// Created by hugo1 on 02.02.2019.
//

#pragma once

#include "logger.hpp"
#include "wrapper/config.hpp"

inline Logger logger;
inline gsl::czstring<> config_file_loc = "config.ini";
inline IniConfig iniConfig;