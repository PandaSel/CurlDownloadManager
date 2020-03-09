#include <iostream>
#include <json/json.h>
#include "DownLoadTask.h"
#include "Singleton.h"
#include "FunHelper.h"
#include "UtilSharedMemory.hpp"

CDownLoadTask::CDownLoadTask()
{
	std::thread t([this]() {
		while (!exitFlag)
		{			
			if (!SharedMemoryExist(CURL_TASK_START_DATA))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			TaskRun runTask;
			if (SharedMemoryRead(CURL_TASK_START_DATA, sizeof(TaskRun), (char*)&runTask))
			{
				std::string srcData = runTask.taskDatas;
				if (srcData.empty())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}

				const auto bufValueLen = static_cast<int>(srcData.length());

				Json::Value value;
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

				if (!reader->parse(srcData.c_str(), srcData.c_str() + bufValueLen, &value, &err))
				{
					curl_trace("Error Data, not json");
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}

				curl_trace("shared_mem read data:%s", srcData.c_str());
				DoDownTask(srcData);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	});
	mt = std::move(t);
}

CDownLoadTask::~CDownLoadTask()
{
	exitFlag = true;

	for (const auto& task : downTasks)
	{
		task.second->curlDownloadPtr->cancelTask = true;
	}

	if (mt.joinable())
	{
		mt.join();
	}

	for (auto& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}

	curl_trace("[Destructor] DownLoadTask has delete!");
}

std::size_t CDownLoadTask::taskNum()
{
	std::lock_guard<std::mutex> lk(taskMutex);
	return downTasks.size();
}

bool CDownLoadTask::DoDownTask(const std::string& sDownInfo)
{
	std::string downLocation;														//定义下载位置
	std::string serverUrl;															//下载url源
	std::string serverName;															//文件名(供显示在下载进度上)
	std::string serverMd5;															//文件MD5
	uint64_t serverSize = 0;														//文件大小
	DownAction action;

	//解析json数据
	Json::Reader reader;  
	Json::Value value;
	if(reader.parse(sDownInfo, value))
	{
		if (!value["location"].isNull() && value["location"].isString())
			downLocation = value["location"].asString();
		if (!value["url"].isNull() && value["url"].isString())
			serverUrl = value["url"].asString();
		if (!value["name"].isNull() && value["name"].isString())
			serverName = value["name"].asString();
		if (!value["md5"].isNull() && value["md5"].isString())
			serverMd5 = value["md5"].asString();
		if (!value["size"].isNull() && value["size"].isUInt64())
			serverSize = value["size"].asUInt64();
		if (!value["action"].isNull() && value["action"].isInt())
			action = static_cast<DownAction>(value["action"].asInt());
	}

	if (serverUrl.empty())
	{
		curl_trace("download url empty");
		return false;
	}

	if (serverName.empty())
	{
		std::size_t rSlashPos = serverUrl.rfind('/');
		serverName = serverUrl.substr(rSlashPos + 1);
	}

	if (downLocation.empty()) {
		downLocation = getExePath();
		downLocation += serverName;
	}

	curl_trace("name:%s, url:%s, md5:%s, size:%ld, location:%s", serverName.c_str(), serverUrl.c_str(), serverMd5.c_str(), serverSize, downLocation.c_str());

	// for downloading
	downLocation.append(".tmp");

	TASKS::iterator it;
	std::unique_lock<std::mutex> lk(taskMutex);
	it = downTasks.find(serverName);
	if (it == downTasks.end()) {
		std::shared_ptr<Task> taskPtr = std::make_shared<Task>();
		CurlDownManager::PTR curlDownloadPtr = std::make_unique<CurlDownManager>();
		taskPtr->curlDownloadPtr = std::move(curlDownloadPtr);
		taskPtr->downInfo.downLocation = downLocation;
		taskPtr->downInfo.stream = nullptr;
		taskPtr->downInfo.serverUrl = serverUrl;
		taskPtr->downInfo.serverName = serverName;
		taskPtr->downInfo.serverMd5 = serverMd5;
		taskPtr->downInfo.serverSize = serverSize;
		taskPtr->action = action;
		downTasks[serverName] = taskPtr;
		lk.unlock();

		std::thread t([taskName = serverName, this](std::weak_ptr<Task> taskPtrWeak) {
			
			std::shared_ptr<Task> taskPtrInn;
			if (!(taskPtrInn = taskPtrWeak.lock()))
			{
				std::cout << "[error] Ptr has free" << std::endl;
				curl_trace("[error] Ptr has free");
				return;
			}
			auto& downPtr = taskPtrInn->curlDownloadPtr;
			auto& downInfo = taskPtrInn->downInfo;
			int curlCode = downPtr->CurlDownLoadFile(taskPtrInn->downInfo);
			if (downPtr->cancelTask)
			{
				curl_trace("has cancel/ pause task,thread ready to exit,taskName:%s", taskName.c_str());
				eraseTask(taskName);
				return;
			}

			if (curlCode != CURLE_OK)
			{
				eraseTask(taskName);
				curl_trace("[downTask] download file failed, code:%d", curlCode);
				return;
			}
			else
			{
				eraseTask(taskName);
				curl_trace("task down,task name is :%s ", taskName.c_str());
			}
		}, taskPtr);
		threads.emplace_back(std::move(t));
	}
	else
	{
		lk.unlock();
		switch (action)
		{
		default:
		case CDownLoadTask::DownAction::INIT:
			curl_trace("Init action, ignore");
			break;
		case CDownLoadTask::DownAction::DOWNLOADING:
			curl_trace("task:%s already start!", serverName);
			break;
		case CDownLoadTask::DownAction::PAUSE:
		case CDownLoadTask::DownAction::CANCEL:
		{
			auto& curlDownloadPtr = it->second->curlDownloadPtr;
			curlDownloadPtr->cancelTask = true;
		}	
		break;
		}
	}
	return true;
}

bool CDownLoadTask::eraseTask(const std::string& taskName)
{
	TASKS::iterator it;
	std::unique_lock<std::mutex> lk(taskMutex);
	it = downTasks.find(taskName);
	if (it != downTasks.end()) {
		downTasks.erase(it++);
		curl_trace("has erase task, task name:%s", taskName.c_str());
		return true;
	}
	curl_trace("could not found task:%s", taskName.c_str());
	return false;
}