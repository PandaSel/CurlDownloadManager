#pragma once
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "commonData.h"
#include "CurlDownManager.h"

class CurlDownManager;
class CDownLoadTask
{
	enum class DownAction
	{
		INIT = 0,
		DOWNLOADING,
		PAUSE,
		CANCEL
	};

	struct Task
	{
		CurlDownManager::PTR curlDownloadPtr;
		FtpFile downInfo;
		DownAction action = DownAction::INIT;
	};
	using TaskPtr = std::shared_ptr<Task>;
	using TASKS = std::unordered_map<std::string, TaskPtr>;

public:
	explicit CDownLoadTask();
	~CDownLoadTask();
	bool DoDownTask(const std::string&);
	std::size_t taskNum();
private:
	bool eraseTask(const std::string& taskName);	

private:
	TASKS downTasks;

	std::thread mt;
	bool exitFlag = false;
// 	CurlDownManager::PTR curlDownloadPtr;
// 	FtpFile ftpFile;
// 	bool m_bPause;
 	std::vector<std::thread> threads;
	std::mutex taskMutex;
};