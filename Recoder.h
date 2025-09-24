#ifndef Recoder_h
#define Recoder_h

#include "MassageQueue.h"
#include <String>
#include "ApiUtils.h"
#include "ApiStructs.h"
#include <thread>

class Recoder
{

public:
    Recoder(MassageQueue *mq, std::string uid, ApiUtils *api);
    ~Recoder();
    void startRecoder();
    bool getRecoding();
    bool getLiving();
    std::string getRoomTitle();
    std::string getRoomId();
    std::string getNickname();
    bool getRoomStatus();
    void updateInfo();
    bool stopFlag_ = false;

private:
    MassageQueue *mq_ = NULL;
    ApiUtils *api_ = NULL;
    std::string uid_;
    bool isRecoding_ = false;
    bool isLiving_ = false;
    LiveInfo liveInfo_;
    std::thread recThread_;
};
#endif