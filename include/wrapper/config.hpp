//
// Created by hugo on 02.02.19.
//
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <INIReader.h>
#include "fwd.hpp"
#include "logger.hpp"

class IniConfig {
public:
    explicit IniConfig(gsl::czstring<> iniFile);

    // Full URI: driver[+transport]://[username@][hostname][:port]/[path][?extraparameters]
    void buildConnURI();

    constexpr auto getConnURI() const noexcept -> const std::string&;

private:
    std::string
            connTRANS,
            connUNAME,
            connHOST,
            connPORT,
            connPATH,
            connEXTP,
            connDRIV;

    std::string connURI;
};