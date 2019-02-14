//
// Created by hugo1 on 12.02.2019.
//

#pragma once

#include <INIReader.h>
#include "logger.hpp"

constexpr auto config_file_loc = "config.ini";

class IniConfig
{
public:
    std::string
            connDRIV,
            connTRANS,
            connUNAME,
            connHOST,
            connPORT,
            connPATH,
            connEXTP,
            connURI,
            http_address,
            http_doc_root,
            http_auth_key,
            httpURI;

    long
            http_port,
            http_threads;

    IniConfig() {
        auto iniFile = config_file_loc;

        INIReader reader(config_file_loc);

        if (reader.ParseError() < 0) {
            logger.warning("Can't load config from ", iniFile);
            logger.info("Using default config");
            logger.info("Libvirtd : qemu:///system");
            logger.info("HTTP Wrapper : http://0.0.0.0:8081/");
        } else
            logger.info("Config loaded from ", iniFile);

        http_address = reader.Get("http_server","address","0.0.0.0");
        http_port = reader.GetInteger("http_server", "port", 8081);
        http_doc_root = reader.Get("http_server", "doc_root", ".");
        http_threads = reader.GetInteger("http_server", "threads", 1);
        http_auth_key = reader.Get("http_server", "auth-key", "");

        connDRIV = reader.Get("libvirtd", "driver", "qemu");
        connTRANS = reader.Get("libvirtd", "transport", "");
        connUNAME = reader.Get("libvirtd", "username", "");
        connHOST = reader.Get("libvirtd", "hostname", "");
        connPORT = reader.Get("libvirtd", "port", "");
        connPATH = reader.Get("libvirtd", "path", "system");
        connEXTP = reader.Get("libvirtd", "extras", "");
        buildConnURI();
        buildHttpURI();
    }

    void buildConnURI() {
        connURI.clear();
        connURI.reserve(64); // Take some extra space, since we're over SSO anyway
        connURI.append(connDRIV);
        if(!connTRANS.empty())
            connURI += '+' + connTRANS;
        connURI += "://";
        if(!connUNAME.empty())
            connURI += connUNAME + '@';
        connURI += connHOST;
        if(!connPORT.empty())
            connURI += ':' + connPORT;
        connURI += '/' + connPATH;
        if(!connEXTP.empty())
            connURI += '?' + connEXTP;
    }
    
    void buildHttpURI() {
        httpURI.clear();
        httpURI.reserve(64);
        httpURI.append("http://");
        httpURI += http_address + ":" + std::to_string(http_port);
    }

    std::string getConnURI() {
        return connURI;
    }
    
    std::string getHttpURI() {
        return httpURI;
    }
};