//
// Created by hugo1 on 12.02.2019.
//

#pragma once

#include <INIReader.h>
#include "logger.hpp"

class IniConfig {
  private:
    std::string default_http_auth_key = "123456789abcdefgh";

  public:
    std::string connDRIV, connTRANS, connUNAME, connHOST, connPORT, connPATH, connEXTP, connURI, http_address, http_doc_root, http_auth_key, httpURI,
        config_file;
    long http_port{}, http_threads{};
    bool http_auth_key_required{};

    IniConfig() = default;

    void init(const std::string& config_file_loc) {
        config_file = config_file_loc;
        INIReader reader(config_file);

        logger.setColored(reader.GetBoolean("wrapperd", "color", false));
        logger.setQuiet(reader.GetBoolean("wrapperd", "quiet", false));
        logger.setDebug(reader.GetBoolean("wrapperd", "debug", false));

        if (reader.ParseError() < 0) {
            logger.warning("Can't load config from ", config_file);
            logger.info("Using default config");
            logger.info("Libvirtd : qemu:///system");
            logger.info("HTTP Wrapper : http://0.0.0.0:8081/");
        } else
            logger.info("Config loaded from ", config_file);

        http_address = reader.Get("http_server", "address", "0.0.0.0");
        http_port = reader.GetInteger("http_server", "port", 8081);
        http_doc_root = reader.Get("http_server", "doc_root", ".");
        http_threads = reader.GetInteger("http_server", "threads", 1);
        http_auth_key_required = reader.GetBoolean("http_server", "auth-key-required", true);
        http_auth_key = reader.Get("http_server", "auth-key", default_http_auth_key);
        if (http_auth_key_required && (http_auth_key == default_http_auth_key))
            logger.warning("Using default HTTP Auth Key: ", default_http_auth_key);

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
        if (!connTRANS.empty())
            connURI += '+' + connTRANS;
        connURI += "://";
        if (!connUNAME.empty())
            connURI += connUNAME + '@';
        connURI += connHOST;
        if (!connPORT.empty())
            connURI += ':' + connPORT;
        connURI += '/' + connPATH;
        if (!connEXTP.empty())
            connURI += '?' + connEXTP;
    }

    void buildHttpURI() {
        httpURI.clear();
        httpURI.reserve(64);
        httpURI.append("http://");
        httpURI += http_address + ":" + std::to_string(http_port);
    }

    const std::string& getConnURI() { return connURI; }

    const std::string& getHttpURI() { return httpURI; }

    const bool& isHTTPAuthRequired() { return http_auth_key_required; }
};