//
// Created by hugo on 07.06.19.
//

#pragma once

#include <cstring>
#include <zconf.h>
#include <cstdlib>

#include "logger.hpp"

class URLParser {
    typedef struct {
        char *name;
        char *value;
    } query;

private:
    const char *url{};
    uint url_len = 0;
    query *queriesPtr{};
    size_t NoE = 0; // Number of elements in the Queries Array

    void fillQuery() {
        uint i = 0;
        queriesPtr = (query *) malloc(sizeof(query *));
        while (url[i] != '?' && i < url_len)
            i++;
        if (i < url_len) {
            uint n = 0;
            i++;
            while (i < url_len) {
                queriesPtr = (query *) realloc(queriesPtr, (n + 1) * sizeof(query));
                queriesPtr[n].name = (char *) malloc(sizeof(char *));
                uint j = 0;
                while (url[i] != '=' && i < url_len) {
                    queriesPtr[n].name = (char *) realloc(queriesPtr[n].name, (j + 1) * sizeof(char *));
                    queriesPtr[n].name[j] = url[i];
                    i++; j++;
                }
                queriesPtr[n].name[j] = '\0';
                i++;
                j = 0;
                queriesPtr[n].value = (char *) malloc(sizeof(char *));
                while (url[i] != '&' && i < url_len) {
                    queriesPtr[n].value = (char *) realloc(queriesPtr[n].value, (j + 1) * sizeof(char *));
                    queriesPtr[n].value[j] = url[i];
                    i++; j++;
                }
                queriesPtr[n].value[j] = '\0';
                n++; i++;
            }
            NoE = n;
        }
    }

public:
    explicit URLParser(const char *URL) {
        setURL(URL);
    }

    ~URLParser() = default;

    void setURL(const char *URL) {
        url = URL;
        url_len = strlen(URL);
        fillQuery();
    }

    char *getQuery(const char *query) {
        for (uint i = 0; i <= (NoE - 1); i++) {
            if (std::strcmp(queriesPtr[i].name, query) == 0)
                return queriesPtr[i].value;
        }
        return nullptr;
    }

    char *getPath() {
        char *str = (char *) malloc(sizeof(char *));
        for (uint i = 0; i <= 6; i++) {
            str = (char *) realloc(str, (i+1) * sizeof(char*));
            str[i] = url[i];
        }
        uint i;
        if (std::strcmp(str, "http://") == 0)
            i = 7;
        else if (std::strcmp(str, "https://") == 0)
            i = 8;
        else
            i = 0;

        while (url[i] != '/' && i < url_len)
            i++;
        char *res = nullptr;
        uint j = 0;
        while (url[i] != '?' && i < url_len) {
            res = (char *) realloc(res, (j+1) * sizeof(char *));
            res[j] = url[i];
            i++; j++;
        }
        if (res)
            res[j] = '\0';
        return res;
    }

    const char *getURL() {
        return url;
    }
};