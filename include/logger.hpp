//
// Created by hugo1 on 02.02.2019.
//
#pragma once

#include <iostream>

class warning : public std::exception
{
public:
    warning(const std::string& msg) {
        std::cout << "WARN: " << msg << std::endl;
    }
    const char* what() {
        return msg.c_str();
    }

private:
    std::string msg;
};

class error : public std::exception
{
public:
    error(const std::string& msg) {
        std::cerr << "ERROR: " << msg << std::endl;
    }
    const char* what() {
        return msg.c_str();
    }

private:
    std::string msg;
};