// main.cpp
#include <iostream>
#include <stdio.h>
#include "json/json.h"
#include "DownLoadTask.h"
#include "UtilSharedMemory.hpp"
#include "FunHelper.h"
#pragma warning (disable:4996)

int main(int argc, char* argv[])
{	
	curl_trace("Task start!");
	for (int i = 1; i < argc; ++i)
	{
		std::cout << argv[i] << std::endl;
	}
	
	if (argc != 2)
	{
		curl_trace("Parameter error! argc:%d", argc);
		MessageBoxA(NULL, "Error parameter", NULL, NULL);
		return 0;
	}
	
	std::string jsonData = argv[1];
	curl_trace(jsonData.c_str());

	HWND hWnd = FindWindowA(NULL, "CurlDownloadManager_");
	if (hWnd != nullptr)
	{
		curl_trace("Windows not empty");
		if (!SharedMemoryExist(CURL_TASK_START_DATA))
		{
			TaskRun run;
			memset(run.taskDatas,0,sizeof(run.taskDatas));
			if (jsonData.length() > sizeof(run.taskDatas))
			{
				curl_trace("Parameter too large:%d", jsonData.length());
				MessageBoxA(NULL, "Parameter too large", NULL, NULL);
				return 0;
			}
			strncpy(run.taskDatas, jsonData.c_str(), jsonData.length());
			curl_trace("Write to shared_mem, taskDatas:%s", run.taskDatas);
			SharedMemoryHandle mSMHandle;
			if (SharedMemoryCreate(CURL_TASK_START_DATA, sizeof(TaskRun), mSMHandle))
			{
				SharedMemoryWrite(CURL_TASK_START_DATA, sizeof(TaskRun), (char*)&run);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				SharedMemoryDestroy(mSMHandle);
			}		
		}
		return 0;
	}
	

// 	Json::Value root;
// 	root["name"] = "2020-01-09.h264";
// 	root["location"] = "D:/abc.h264";
// 	root["url"] = "http://10.2.35.136:8081/api/hdfs/singlevideo?channel=video1&date=2020-01-09&time=05-23";
// 	root["md5"] = "";
// 	root["size"] = 0;


// 	Json::Value root;
// 	root["name"] = "2020-01-09.h264";
// 	root["location"] = "D:/abc.h264";
// 	root["url"] = "http://10.2.35.136:8081/api/hdfs/singlevideo?channel=video1&date=2020-01-09&time=05-23";
// 	Json::StyledWriter writer; 	
// 	std::string jsonData = writer.write(root);

	{
		std::unique_ptr<CDownLoadTask> downTaskPtr = std::make_unique<CDownLoadTask>();
		downTaskPtr->DoDownTask(jsonData);

		SetConsoleTitleA("CurlDownloadManager_");
		std::this_thread::sleep_for(std::chrono::milliseconds(40));

		while (downTaskPtr->taskNum() != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		curl_trace("No task run, program ready to exit!");
	}	

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return 0;
}