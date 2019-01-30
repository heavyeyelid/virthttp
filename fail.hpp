//
// Created by hugo on 30.01.19.
//
#pragma once

#include <iostream>
#include <boost/system/system_error.hpp>

void fail(boost::system::error_code ec, const char* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}