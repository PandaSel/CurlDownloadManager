#pragma once
#include <string>

// 获取启动任务参数
#define CURL_TASK_START_DATA "CURL_TASK_START_DATA"
// 推送速率,进度等至共享内存
#define CURL_TASK_POST_DATA "CURL_TASK_POST_DATA"


struct FtpFile {
	FtpFile() :stream(nullptr) {}
	std::string downLocation;														//定义下载位置
	FILE * stream;																	//文件流
	std::string serverUrl;															//下载url源
	std::string serverName;															//文件名(供显示在下载进度上)
	std::string serverMd5;															//文件MD5
	int64_t serverSize;																//文件大小
	
	bool empty()
	{
		return (serverUrl.empty() || serverName.empty() || serverMd5.empty() || serverSize == 0 );
	}
private:
	FtpFile(const FtpFile&);
	FtpFile& operator = (const FtpFile& other);
};

struct TaskRun
{
	char taskDatas[1024 * 2];	// json数组
};

struct TaskCallBackData 
{
	char taskCallbackDatas[1024];	// json数组
};
