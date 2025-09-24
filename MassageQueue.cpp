#include "MassageQueue.h"
#include <mutex>
#include <queue>
MassageQueue::MassageQueue()
{
}

MassageQueue::~MassageQueue()
{
}

MessageType MassageQueue::getMessage()
{
    mutex.lock();

    if (q.empty())
    {
        return MSG_NONE;
    }
    MessageType msg = q.front();
    q.pop();

    mutex.unlock();

    return msg;
}

int MassageQueue::pushMessage(MessageType msg)
{
    mutex.lock();
    q.push(msg);
    mutex.unlock();
    return 0;
}
