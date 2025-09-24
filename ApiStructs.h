#ifndef APISTRUCTS_H
#define APISTRUCTS_H
#include <string>

struct LiveInfo
{
    std::string room_id;
    std::string user_id;
    std::string ttwid;
    std::string room_status;
    std::string room_title;
    std::string nickname;
    std::string http_flv_url;
    std::string web_rid;
};

#endif