#include "ApiUtils.h"
#include <curl/curl.h>
#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <RE2/re2.h>

using json = nlohmann::json;

ApiUtils::ApiUtils()
{
}

ApiUtils::ApiUtils(std::string cookie)
{
    cookie_ = cookie;
}

ApiUtils::~ApiUtils()
{
}
// html回调函数
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}
// Header回调函数
static size_t HeaderCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}
std::map<std::string, std::string> parse_cookie(const std::string &cookie_str)
{
    std::map<std::string, std::string> cookies;
    std::regex re(R"(([^=;\s]+)=([^;]+))");
    auto begin = std::sregex_iterator(cookie_str.begin(), cookie_str.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it)
    {
        cookies[it->str(1)] = it->str(2);
    }
    return cookies;
}
std::string urlAddParam(std::string &url, const std::vector<std::pair<std::string, std::string>> &params)
{
    std::string url_with_params = url;
    bool flag = true;
    for (auto &param : params)
    {
        if (flag)
        {
            url_with_params += param.first + "=" + param.second;
            flag = false;
        }
        else
        {
            url_with_params += "&" + param.first + "=" + param.second;
        }
    }
    return url_with_params;
}
int ApiUtils::dyGetLiveInfo(std::string &uid, LiveInfo &info)
{
    CURL *curl = NULL;
    CURLcode res;
    std::string readBuffer;
    std::string headerBuffer;

    curl = curl_easy_init();
    if (curl == NULL)
    {
        std::cout << "curl_easy_init() failed" << std::endl;
        return -1;
    }

    std::string url = uid;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:117.0) Gecko/20100101 Firefox/117.0");
    headers = curl_slist_append(headers, "cache-control: no-cache");
    headers = curl_slist_append(headers, "pragma: no-cache");
    headers = curl_slist_append(headers, "Sec-CH-UA: \"Chromium\";v=\"139\", \"Microsoft Edge\";v=\"139\", \"Not.A/Brand\";v=\"8\"");
    headers = curl_slist_append(headers, "sec-ch-ua-mobile: ?0");
    headers = curl_slist_append(headers, "sec-ch-ua-platform: \"Windows\"");
    headers = curl_slist_append(headers, "Sec-Fetch-Dest: empty");
    headers = curl_slist_append(headers, "Sec-Fetch-Mode: cors");
    headers = curl_slist_append(headers, "Sec-Fetch-Site: same-origin");
    headers = curl_slist_append(headers, "priority: u=1,i");
    headers = curl_slist_append(headers, "accept-language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6");
    headers = curl_slist_append(headers, "Accept: application/json, text/plain, */*");
    headers = curl_slist_append(headers, "Referer: https://live.douyin.com/");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_.c_str()); // 传入 cookie
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);      // 开启 cookie 引擎
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookie.txt"); // 保存 cookie 持久化
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerBuffer);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "curl_easy_perform() failed" << std::endl;
        return -1;
    }

    // std::ofstream outfile("readbuffer_output.txt", std::ios::out | std::ios::trunc);
    // if (outfile.is_open())
    // {
    //     outfile << readBuffer;
    //     outfile.close();
    //     // std::cout << "readBuffer 已保存到 readbuffer_output.txt" << std::endl;
    // }
    // else
    // {
    //     // std::cerr << "无法打开文件保存 readBuffer" << std::endl;
    // }

    std::smatch match;
    std::string json_str, cleaned_string;
    // std::cout << "start analysis" << std::endl;

    // 获取 ttwid
    RE2 rettwid_re(R"(ttwid=([^;]+))");
    if (!RE2::PartialMatch(headerBuffer, rettwid_re, &info.ttwid))
    {
        std::cout << "ttwid not found" << std::endl;
        return -1;
    }

    // 获取 room_id 为开播为NULL
    RE2 reroomId_re(R"(\\\"roomId\\\":\\\"(\d+)\\\")");
    if (!RE2::PartialMatch(readBuffer, reroomId_re, &info.room_id))
    {
        //  未开播
    }

    // 获取 user_id
    RE2 reuserId_re(R"(\\\"user_unique_id\\\":\\\"(\d+)\\\")");
    if (!RE2::PartialMatch(readBuffer, reuserId_re, &info.user_id))
    {
        std::cout << "user_id not found" << std::endl;
        return -1;
    }

    // 获取 web_rid
    RE2 rewebRid_re(R"(\\\"web_rid\\\":\\\"(\d+)\\\")");
    if (!RE2::PartialMatch(readBuffer, rewebRid_re, &info.web_rid))
    {
        std::cout << "web_rid not found" << std::endl;
        return -1;
    }

    // 获取 nickname
    RE2 renickname_re(R"(\\\"nickname\\\":\\\"([^\\\"]+)\\\",\\\"avatar_thumb\\\")");
    if (!RE2::PartialMatch(readBuffer, renickname_re, &info.nickname))
    {
        std::cout << "nickname not found" << std::endl;
        return -1;
    }

    // 获取room_status, room_title, http_flv_url  未开播为NULL
    RE2 roomInfo_re(R"(\\\"roomInfo\\\":\{\\\"room\\\":\{\\\"id_str\\\":\\\".*?\\\",\\\"status\\\":(.*?),\\\"status_str\\\":\\\".*?\\\",\\\"title\\\":\\\"(.*?)\\\")");
    if (!RE2::PartialMatch(readBuffer, roomInfo_re, &info.room_status, &info.room_title))
    {
    }
    if (info.room_status == "2")
    {
        RE2 stream_url(R"(\{\\\"main\\\":\{\\\"flv\\\":\\\"([^\"]+)\\\")");
        if (!RE2::PartialMatch(readBuffer, stream_url, &info.http_flv_url))
        {
            std::cout << "http_flv_url not found" << std::endl;
            return -1;
        }
        std::string::size_type pos = 0;
        const std::string from = "\\u0026";
        const std::string to = "&";
        while ((pos = info.http_flv_url.find(from, pos)) != std::string::npos)
        {
            info.http_flv_url.replace(pos, from.length(), to);
            pos += to.length();
        }
    }

    // std::cout << "user_id: " << info.user_id << std::endl;
    // std::cout << "ttwid: " << info.ttwid << std::endl;
    // std::cout << "room_id: " << info.room_id << std::endl;
    // std::cout << "nickname: " << info.nickname << std::endl;
    // std::cout << "room_status: " << info.room_status << std::endl;
    // std::cout << "room_title: " << info.room_title << std::endl;
    // std::cout << "web_rid: " << info.web_rid << std::endl;
    // std::cout << "http_flv_url: " << info.http_flv_url << std::endl;

    return 0;
}