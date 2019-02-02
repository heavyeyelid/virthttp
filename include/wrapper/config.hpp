//
// Created by hugo on 02.02.19.
//
#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <INIReader.h>

#include "../logger.hpp"

class IniConfig {
public:
    void setGlobalConfigFromIni(std::string iniFile) {
        INIReader reader(iniFile);

        if (reader.ParseError() < 0) {
            std::cout << "Can't load libvirtd config from " + iniFile << std::endl;
            std::cout << "Using default libvirtd config (qemu:///system)" << std::endl;
        } else std::cout << "LibVirtD config loaded from " + iniFile << std::endl;

        connDRIV = reader.Get("libvirtd", "driver", "qemu");
        connTRANS = reader.Get("libvirtd", "transport", "");
        connUNAME = reader.Get("libvirtd", "username", "");
        connHOST = reader.Get("libvirtd", "hostname", "");
        connPORT = reader.Get("libvirtd", "port", "");
        connPATH = reader.Get("libvirtd", "path", "system");
        connEXTP = reader.Get("libvirtd", "extras", "");
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

    std::string getConnURI() {
        return connURI;
    }

private:
    std::string
            connTRANS,
            connUNAME,
            connHOST,
            connPORT,
            connPATH,
            connEXTP,
            connDRIV;

    char* connURI;
};