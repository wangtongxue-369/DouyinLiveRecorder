#include "MainManager.h"
#include <iostream>
MainManager::MainManager()
{
    mq = new MassageQueue();
    api_ = new ApiUtils();
}
MainManager::MainManager(std::string cookie)
{
    mq = new MassageQueue();
    api_ = new ApiUtils(cookie);
    cookie_ = cookie;
}
MainManager::~MainManager()
{
    delete mq;
    delete api_;
}

void MainManager::addRecoder(std::string url)
{
    recoders.push_back(new Recoder(mq, url, api_));
}

void MainManager::run()
{
    while (running_)
    {
        updateInfo();
        std::cout << "---------------DouyinLiveRecoder----------------" << std::endl;
        std::cout << "Recording List" << std::endl;
        showInfo();
        for (int i = 1; i <= 30; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (!running_)
            {
                std::cout << "All Recording Stopped" << std::endl;
                return;
            }
        }
    }
    std::cout << "All Recording Stopped" << std::endl;
}

void MainManager::start()
{
    running_ = true;
    thread_ = std::thread(&MainManager::run, this);
}

void MainManager::stop()
{
    running_ = false;
    for (auto it : recoders)
    {
        it->stopFlag_ = true;
    }
}

void MainManager::showInfo()
{
    if (recoders.size() == 0)
    {
        std::cout << "No Recording" << std::endl;
        return;
    }
    for (auto it : recoders)
    {
        std::cout << "nickname: " << it->getNickname() << " roomTitle: " << it->getRoomTitle() << " liveStatus: " << (it->getRoomStatus() ? " Live " : " offLive ") << std::endl;
    }
}

void MainManager::updateInfo()
{
    for (auto it : recoders)
    {
        it->updateInfo();
        if (running_ == false)
        {
            return;
        }
    }
}
