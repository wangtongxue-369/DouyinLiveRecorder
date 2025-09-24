#ifndef MASSAGE_QUEUE_H
#define MASSAGE_QUEUE_H

#include <queue>
#include <mutex>
enum MessageType
{
    MSG_QUIE,
    MSG_START,
    MSG_NONE
};
class MassageQueue
{
public:
    MassageQueue();
    ~MassageQueue();
    MessageType getMessage();
    int pushMessage(MessageType msg);

private:
    std::mutex mutex;
    std::queue<MessageType> q;
};
#endif