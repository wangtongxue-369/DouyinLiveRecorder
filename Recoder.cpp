#include "Recoder.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <curl/curl.h>
Recoder::Recoder(MassageQueue *mq, std::string uid, ApiUtils *api)
{
    this->mq_ = mq;
    this->uid_ = uid;
    this->api_ = api;
    this->stopFlag_ = false;
    api->dyGetLiveInfo(uid_, liveInfo_);
    isLiving_ = liveInfo_.room_status == "2";
    if (isLiving_)
    {
        startRecoder();
    }
}

Recoder::~Recoder()
{
}
struct RecoderWriteContext
{
    Recoder *self;
    FILE *fp;
};
size_t writeData(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    RecoderWriteContext *ctx = static_cast<RecoderWriteContext *>(userdata);
    if (ctx->self->stopFlag_)
    {
        ctx->self->stopFlag_ = false;
        return 0; // 返回 0 => 中止传输
    }
    FILE *fp = ctx->fp;
    return fwrite(ptr, size, nmemb, fp);
}
void Recoder::startRecoder()
{
    if (!isLiving_)
    {
        return;
    }
    if (isRecoding_)
    {
        return;
    }
    std::string streamUrl = liveInfo_.http_flv_url;
    // 添加时间
    time_t now = time(0);
    std::string filename = liveInfo_.room_id + " - " + std::to_string(now) + ".flv";
    std::string path = liveInfo_.room_id + "/";
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }
    recThread_ = std::thread([this, streamUrl, filename, path]()
                             {
        isRecoding_ = true;
        std::cout << "start recoding: "<<liveInfo_.nickname << std::endl;
        std::cout<< "file will be saved to "<<path + filename<<std::endl;

        CURL *curl = curl_easy_init();
        if (!curl)
        {
            std::cerr << "curl init failed" << std::endl;
            isRecoding_ = false;
            return;
        }

        FILE *fp = fopen((path + filename).c_str(), "wb");
        if (!fp)
        {
            std::cerr << "fail to open file" << filename << std::endl;
            curl_easy_cleanup(curl);
            isRecoding_ = false;
            return;
        }
        RecoderWriteContext context = {this, fp};

        curl_easy_setopt(curl, CURLOPT_URL, streamUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &context);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);

        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);   // 低于 1 字节/秒
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 20L);   // 连续 20 秒
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_WRITE_ERROR){

        }else if (res != CURLE_OK)
        {
            std::cerr << "curl download failed " << curl_easy_strerror(res) << std::endl;
        }

        fclose(fp);
        curl_easy_cleanup(curl);

        isRecoding_ = false;
        std::cout <<"over recoding " <<this->liveInfo_.nickname<<" "<< filename << std::endl; });
}
bool Recoder::getRecoding()
{
    return isRecoding_;
}

bool Recoder::getLiving()
{
    return isLiving_;
}

std::string Recoder::getRoomTitle()
{
    return liveInfo_.room_title;
}

std::string Recoder::getRoomId()
{
    return liveInfo_.room_id;
}

std::string Recoder::getNickname()
{
    return liveInfo_.nickname;
}

bool Recoder::getRoomStatus()
{
    isLiving_ = liveInfo_.room_status == "2";
    return liveInfo_.room_status == "2";
}

void Recoder::updateInfo()
{
    LiveInfo now = liveInfo_;
    api_->dyGetLiveInfo(uid_, now);
    if (liveInfo_.room_status == "4" && now.room_status == "2")
    {
        // 开始
        startRecoder();
    }
}
