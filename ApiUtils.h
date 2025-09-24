#ifndef APIUTILS_H
#define APIUTILS_H

#include <String>
#include "ApiStructs.h"

class ApiUtils
{
public:
    ApiUtils();
    ApiUtils(std::string cookie);
    ~ApiUtils();
    int dyGetLiveInfo(std::string &uid, LiveInfo &info);
    int dyGetLiveInfo2(std::string &uid, LiveInfo &info);
    std::string cookie_;
};

#endif