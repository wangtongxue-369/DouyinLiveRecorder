#ifndef MAIN_MANAGER_H
#define MAIN_MANAGER_H
#include "MassageQueue.h"
#include "Recoder.h"
#include <vector>
#include "ApiUtils.h"
#include <string>
class MainManager
{
public:
	MainManager();
	MainManager(std::string cookie);
	~MainManager();
	void addRecoder(std::string url);
	void run();
	void start();
	void stop();
	void showInfo();
	void updateInfo();

	MassageQueue* mq = NULL;
	std::vector<Recoder*> recoders;
	ApiUtils* api_ = NULL;
	std::string cookie_;
	std::thread thread_;
private:

	bool running_ = false;
};

#endif