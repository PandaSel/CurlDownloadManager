#pragma once

#include <curl/curl.h>
#include "commonData.h"
#include "UtilSharedMemory.hpp"

class CurlDownManager
{
public:
	using PTR = std::unique_ptr<CurlDownManager>;
public:
	CurlDownManager();
	
	~CurlDownManager();

	CURLcode CurlDownLoadFile(const FtpFile& ftpFile);

private:
	void destoryresouce();
	int CheckMd5(const std::string& serverMD5);

public:
	std::string downLocation;
	std::string serverUrl;
	std::string serverName;
	std::string serverMd5;
	int64_t serverSize;
	FILE* stream = nullptr;


	CURL *curlFile;

	long	curReadSize = 0;									//当前下载
	long	localSize = 0;


	DWORD m_dCountNow = 0;										//now - last一个回调用时（ms）
	DWORD m_dCountLast = 0;

	double m_dTimes = 0.0;										//大于500ms时间差
	double m_dLenth = 0.0;										//大于500ms量差

	double m_dDownNum_Now = 0.0;								//now - last 一个回调下载量（字节）
	double m_dDownNum_Last = 0.0;

	bool cancelTask = false;									//是否点击取消按钮
};