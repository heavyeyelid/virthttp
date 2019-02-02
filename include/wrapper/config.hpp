//
// Created by hugo on 02.02.19.
//
#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "../INIReader.h"

class IniConfig {
public:
    const std::string config_file_loc = "config.ini";

    std::string
            connTRANS,
            connUNAME,
            connHOST,
            connPORT,
            connPATH,
            connEXTP,
            connDRIV;

    char* connURI;

    void setGlobalConfigFromIni() {
        INIReader reader(config_file_loc);

        if (reader.ParseError() < 0) {
            std::cerr << "Can't load " << config_file_loc << std::endl;
        }

        std::cout << "Config loaded from " << config_file_loc << std::endl;
        connDRIV = reader.Get("libvirtd", "driver", "UNKNOWN");
        connTRANS = reader.Get("libvirtd", "transport", "UNKNOWN");
        connUNAME = reader.Get("libvirtd", "username", "UNKNOWN");
        connHOST = reader.Get("libvirtd", "hostname", "UNKNOWN");
        connPORT = reader.Get("libvirtd", "port", "UNKNOWN");
        connPATH = reader.Get("libvirtd", "path", "UNKNOWN");
        connEXTP = reader.Get("libvirtd", "extras", "UNKNOWN");

        if(connDRIV.empty() || connPATH.empty())
            std::cerr << "Please specify at least the driver and path in " << config_file_loc;
    }

    // Full URI: driver[+transport]://[username@][hostname][:port]/[path][?extraparameters]
    void setConnURI() {
        std::string str = connDRIV;
        if(!connTRANS.empty())
            str = str + "+" + connTRANS;
        str = str + "://";
        if(!connUNAME.empty())
            str = str + connUNAME + "@";
        if(!connHOST.empty())
            str = str + connHOST;
        if(!connPORT.empty())
            str = str + ":" + connPORT;
        str = str + "/" + connPATH;
        if(!connEXTP.empty())
            str = str + "?" + connEXTP;
        connURI = const_cast<char *>(str.c_str());
    }

    IniConfig() {
        setGlobalConfigFromIni();
        setConnURI();
    }
};