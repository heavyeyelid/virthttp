//
// Created by hugo on 02.02.19.
//
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <INIReader.h>
#include "logger.hpp"

class IniConfig : public INIReader {
public:
    void setGlobalConfigFromIni(std::string iniFile) {
        INIReader reader(iniFile);

        if (reader.ParseError() < 0) {
            logger.warning("Can't load libvirtd config from " + iniFile);
            logger.info("Using default libvirtd config (qemu:///system)");
        } else logger.info("LibVirtD config loaded from " + iniFile);

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
        str = str + connHOST;
        if(!connPORT.empty())
            str = str + ":" + connPORT;
        str = str + "/" + connPATH;
        if(!connEXTP.empty())
            str = str + "?" + connEXTP;
        connURI = (str.c_str());
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

    const char* connURI;
};

inline IniConfig iniConfig;