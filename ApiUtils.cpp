#include "ApiUtils.h"
#include <curl/curl.h>
#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>

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

    std::regex ttwid_re("ttwid=([^;]+)");
    if (std::regex_search(headerBuffer, match, ttwid_re))
    {
        info.ttwid = match[1];
    }

    std::regex roomId_re(R"(\\\"roomId\\\":\\\"(\d+)\\\")");
    if (std::regex_search(readBuffer, match, roomId_re))
    {
        info.room_id = match[1];
    }

    std::regex userId_re(R"(\\\"user_unique_id\\\":\\\"(\d+)\\\")");
    if (std::regex_search(readBuffer, match, userId_re))
    {
        info.user_id = match[1];
    }

    std::regex webRid_re(R"(\\\"web_rid\\\":\\\"(\d+)\\\")");
    if (std::regex_search(readBuffer, match, webRid_re))
    {
        info.web_rid = match[1];
    }

    std::regex nickname_re(R"(\\\"nickname\\\":\\\"([^\\\"]+)\\\",\\\"avatar_thumb\\\")");
    if (std::regex_search(readBuffer, match, nickname_re))
    {
        info.nickname = match[1];
    }

    std::regex roomInfo_re(R"(\\\"roomInfo\\\":\{\\\"room\\\":\{\\\"id_str\\\":\\\".*?\\\",\\\"status\\\":(.*?),\\\"status_str\\\":\\\".*?\\\",\\\"title\\\":\\\"(.*?)\\\")");
    if (std::regex_search(readBuffer, match, roomInfo_re))
    {
        info.room_status = match[1];
        info.room_title = match[2];
    }
    if (info.room_status == "2")
    {
        std::regex stream_url(R"(\{\\\"main\\\":\{\\\"flv\\\":\\\"([^\"]+)\\\")");
        if (std::regex_search(readBuffer, match, stream_url))
        {
            info.http_flv_url = match[1];
            std::string::size_type pos = 0;
            const std::string from = "\\u0026";
            const std::string to = "&";
            while ((pos = info.http_flv_url.find(from, pos)) != std::string::npos)
            {
                info.http_flv_url.replace(pos, from.length(), to);
                pos += to.length();
            }
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

int ApiUtils::dyGetLiveInfo2(std::string &uid, LiveInfo &info)
{
    std::cout << "in" << std::endl;
    CURL *curl = NULL;
    CURLcode res;
    std::string readBuffer;

    std::string url = "https://live.douyin.com/webcast/room/web/enter/?";
    std::cout << "url: " << url << std::endl;
    std::vector<std::pair<std::string, std::string>> params = {
        {"aid", "6383"},
        {"app_name", "douyin_web"},
        {"live_id", "1"},
        {"device_platform", "web"},
        {"language", "zh-CN"},
        {"enter_from", "web_live"},
        {"cookie_enabled", "true"},
        {"screen_width", "2048"},
        {"screen_height", "1152"},
        {"browser_language", "zh-CN"},
        {"browser_platform", "Win32"},
        {"browser_name", "Chrome"},
        {"browser_version", "139.0.0.0"},
        {"web_rid", uid.c_str()},
        {"room_id_str", "7549069523173903113"},
        {"enter_source", ""},
        {"is_need_double_stream", "false"},
        {"insert_task_id", ""},
        {"live_reason", ""},
        {"msToken", "iZlrNf7cYr8bX8HPtbADdADkbXpvm-yzxnkFCeV0yC_YBuKzOSwr9-auAUrFSQGqsAGfDsEXB0s4VpXqViHfbuhBRTEUQawQ3plFsx6UmHVa0JmIy0y4ZcSPOjpXrkxZ-lea1VD9h1f3vmeJzCZq_aJis632PJb0q1FYu2CH-t8fTgdBVF7I5PM%3D"},
        {"a_bogus", "dj4RgetwmxQbPdMGYOc3tn2UhqIlNTWyYBToRcxK9Puwc7eGh8NtLNbQJoqu4DDhJSpTio3HnjlMbEVczdUT1MnkumkvSQTbHtdnVtsoMHH4TPJg9p8MCDGELksTlATT0CcGidEIIUZrggQ3hH9iAVBG7OFnBYbBbNFUd2bbJ9VNVCjH9xdCeBSdKhv%2F"},
    };
    url = urlAddParam(url, params); // 带参数的url
    std::cout << url << std::endl;
    curl = curl_easy_init();
    if (curl == NULL)
    {
        std::cout << "curl_easy_init() failed" << std::endl;
        return -1;
    }
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/139.0.0.0 Safari/537.36");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2");
    headers = curl_slist_append(headers, ("Referer: https://live.douyin.com/" + uid).c_str());
    headers = curl_slist_append(headers, ("Cookie: " + cookie_).c_str());
    // std::cout << "cookie: " << cookie_ << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    std::cout << "!!!" << std::endl;
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK)
    {
        std::cout << "curl_easy_perform() failed" << std::endl;
        return -1;
    }
    std::cout << readBuffer << std::endl;

    return 0;
}
