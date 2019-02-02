//
// Created by _as on 2019-02-02.
//

#pragma once

#include <gsl/gsl>
#include "../config.hpp"
#include "fwd.hpp"

IniConfig::IniConfig(gsl::czstring<> iniFile) {
  const INIReader reader(iniFile);

  if (reader.ParseError() < 0) {
    logger.warning("Can't load libvirtd config from ", iniFile);
    logger.info("Using default libvirtd config (qemu:///system)");
  } else
    logger.info("LibVirtD config loaded from ", iniFile);

  connDRIV = reader.Get("libvirtd", "driver", "qemu");
  connTRANS = reader.Get("libvirtd", "transport", "");
  connUNAME = reader.Get("libvirtd", "username", "");
  connHOST = reader.Get("libvirtd", "hostname", "");
  connPORT = reader.Get("libvirtd", "port", "");
  connPATH = reader.Get("libvirtd", "path", "system");
  connEXTP = reader.Get("libvirtd", "extras", "");
  buildConnURI();
}

void IniConfig::buildConnURI() {
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

constexpr auto IniConfig::getConnURI() const noexcept -> const std::string& {
  return connURI;
}