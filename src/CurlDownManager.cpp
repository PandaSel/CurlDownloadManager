#include "CurlDownManager.h"
#include <iostream>
#include "json/json.h"
#include "FunHelper.h"
#include <chrono>

CurlDownManager::CurlDownManager()
{

}

CurlDownManager::~CurlDownManager()
{
}

void CurlDownManager::destoryresouce()
{
	if(stream)
	{
		fflush(stream);
		//关闭文件流
		int fclosecode =  fclose(stream);
		if (fclose == 0)
		{
			stream = nullptr;
		}
		curl_trace("[downloadManager] fclose return value is:%d", fclosecode);
	}

	if (curlFile) {
		curl_easy_cleanup(curlFile);
	}	
}

size_t downLoadCallback(void *buffer, size_t size, size_t nmemb, void *stream)
{
	unsigned long len = static_cast<unsigned long>(size * nmemb);
	CurlDownManager *curlDownload = (CurlDownManager*)stream;
	if (!curlDownload) {
		curl_trace("[downloadManager] failed! this has free");
		return -1;
	}

	curlDownload->curReadSize += len;

	curlDownload->m_dCountNow = GetTickCount();
	curlDownload->m_dDownNum_Now = (double)curlDownload->curReadSize + (double)curlDownload->localSize;
	if (curlDownload->m_dCountLast == 0)
	{
		curlDownload->m_dCountLast = curlDownload->m_dCountNow;
		curlDownload->m_dDownNum_Last = curlDownload->m_dDownNum_Now;
	}
	else
	{
		curlDownload->m_dTimes = curlDownload->m_dCountNow - curlDownload->m_dCountLast;
		if (curlDownload->m_dTimes >= 500)
		{
			curlDownload->m_dCountLast = curlDownload->m_dCountNow;
			curlDownload->m_dLenth = curlDownload->m_dDownNum_Now - curlDownload->m_dDownNum_Last;
			curlDownload->m_dDownNum_Last = curlDownload->m_dDownNum_Now;

			int nPos = 0;
			if (curlDownload->serverSize == 0)
				nPos = 100;
			else
				nPos = (int) ( (((double)curlDownload->curReadSize + (double)curlDownload->localSize)/(double)curlDownload->serverSize)*100 );

			if (nPos > 100)
				nPos = 100;

			Json::FastWriter fastwrite;
			Json::Value map;
			map["sTaskName"] = curlDownload->serverName;
			map["dTaskSpeed"] = (double)curlDownload->m_dLenth*1000/1024/curlDownload->m_dTimes;
			map["dTaskLen"] = (double)curlDownload->curReadSize + (double)curlDownload->localSize;
			map["dTaskFullLen"] = (double)curlDownload->serverSize;
			map["nPos"] = nPos;
			std::string jsonData = fastwrite.write(map);
			curlDownload->m_dLenth = 0;
			curlDownload->m_dTimes = 0;

			// write to shared_memory
			TaskCallBackData callbackData;
			memset(callbackData.taskCallbackDatas, 0, sizeof(callbackData));
			strncpy(callbackData.taskCallbackDatas, jsonData.c_str(), jsonData.length());
			if (!SharedMemoryExist(CURL_TASK_POST_DATA))
			{
				SharedMemoryHandle mSMHandle;
				SharedMemoryCreate(CURL_TASK_POST_DATA, sizeof(TaskCallBackData), mSMHandle);
			}

			SharedMemoryWrite(CURL_TASK_POST_DATA, sizeof(TaskCallBackData), (char*)&callbackData);

//			::SendMessage(FindWindowA("DownTask2Ui",NULL),DOWN_TASK_PROGRESS, (WPARAM)jsonData.c_str() ,NULL);
		}
	}

	if (!curlDownload->stream)
	{
 		curlDownload->stream = fopen(curlDownload->downLocation.c_str(), "a+b");//打开文件进行写入
		if(!curlDownload->stream)
		{
			return -1;
		}
	}
	
	if (curlDownload->cancelTask)
		return -1;
	
	return fwrite(buffer, size, nmemb, curlDownload->stream);
}

size_t process_data(void *buffer, size_t size, size_t nmemb, void *stream)
{
	return -1;
}

size_t process_header_data(void *buffer, size_t size, size_t nmemb, void *stream)
{
	if (stream == nullptr)
	{
		return -1;
	}
	std::string *sGetdate = (std::string*)stream;
	sGetdate->append((char*)buffer, strlen((char*)buffer));
	return size * nmemb;
}

// debug method
static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp) {
	const char *text;  
	(void)handle;
	(void)userp;

	switch (type) {
	case CURLINFO_TEXT:    
//		fprintf(stderr, "== Info: %s", data);  
	default:
		return 0;
	case CURLINFO_HEADER_OUT:    
		text = "=> Send header ";  
		break;  
	case CURLINFO_DATA_OUT:   
		text = "=> Send data "; 
		break;  
	case CURLINFO_SSL_DATA_OUT: 
		text = "=> Send SSL data ";   
		break;  
	case CURLINFO_HEADER_IN:    
		text = "<= Recv header ";  
		break;  
	case CURLINFO_DATA_IN:    
		text = "<= Recv data ";
		break;  
	case CURLINFO_SSL_DATA_IN:  
		text = "<= Recv SSL data ";
		break;
	}
	std::cout << text << data; 
	return 0;
}

long getDownloadFileLength(const char *url)//为了得到下载文件的大小
{
	std::string headerDatas;
	long fileLength = 0;
//	curl_slist* headers = NULL;
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HEADER, true);
//	headers = curl_slist_append(headers, "Content-Type:application/x-www-form-urlencoded");
//	headers = curl_slist_append(headers, "Accept: application/octet-stream");
//	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_NOBODY, true);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
//	curl_easy_setopt(curl, CURLOPT_POST, false);
//	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
//	curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &strOut);

	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &process_header_data);	//对返回的数据进行操作的函数地址 
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerDatas);
	CURLcode res = curl_easy_perform(curl);

	if (res == CURLE_OK)
	{
//		curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &headSize);
//		std::cout << headSize << std::endl;
		std::stringstream os(headerDatas);
		std::string line;
		const char* findStr = "Content-Length:";
		while (std::getline(os, line))
		{
			const char* ptr = strstr(line.c_str(), findStr);
			if (ptr != nullptr)
			{
				std::string fileLen = line.substr(strlen(findStr));
				fileLength = std::stol(fileLen);
				std::cout << "[download] get server file size:" << fileLength << std::endl;
				curl_trace("[download] get server file size :%ld", fileLength);
				break;
			}
		}
	}

	curl_easy_cleanup(curl);
//	curl_slist_free_all(headers);
 	return fileLength;

	/*
	long filesize = 0;
	CURLcode res;
    CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &process_data);
//	curl_easy_setopt(handle, CURLOPT_TIMEOUT, 10);  //设置访问的超时
	// 设置重定向的最大次数  
	curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 3);  
	// 设置301、302跳转跟随location  
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(handle,CURLOPT_HTTP_VERSION , CURL_HTTP_VERSION_1_0);
	// 禁止SSL验证
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
	res = curl_easy_perform(handle);
	
	if(CURLE_OK == res || res == CURLE_WRITE_ERROR)
	{
		// 设置重定向的最大次数  
		curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 3);
		res = curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
		if((CURLE_OK == res) && (filesize > 0))
		{
			curl_easy_cleanup(handle);
			return filesize;//表明正确的获取文件大小
		}
	}
//	curl_easy_cleanup(handle);
	return 0;
	*/
}

CURLcode CurlDownManager::CurlDownLoadFile(const FtpFile& ftpFile)
{
	downLocation = ftpFile.downLocation;
	serverUrl = ftpFile.serverUrl;
	serverName = ftpFile.serverName;
	serverMd5 = ftpFile.serverMd5;
	serverSize = ftpFile.serverSize;

	std::string diskLocation = downLocation.substr(0, downLocation.length() - 4);
	// mkdir -p
	CreateDirectory(downLocation);

	if (serverSize == 0)
	{
		long size = getDownloadFileLength(serverUrl.c_str());
		if (size <= 0) {
			serverSize = 0;
		}
		else {
			serverSize = size;
		}
	}

	curl_trace("[downloadManager] fileSize is:%ld, md5:%s", serverSize, serverMd5.c_str());
	
	//================断点续载===================  
	
	// 判断本地文件存在则返回成功
	if (FileExist(diskLocation, false))
	{
		if (!serverMd5.empty()) 
		{
			std::string md5 = BuildFileMd5(diskLocation);
			if (serverMd5 == md5)
			{
				curl_trace("[downloadManager] success local file exist, md5 equal");
				return CURLE_OK;
			}
			else
			{
				remove(diskLocation.c_str());
			}
		}
		else if (serverSize != 0)
		{
			auto localSize = GetLocalFileLenth(diskLocation.c_str());
			if (localSize == serverSize)
			{
				curl_trace("[downloadManager] success local file exist, size equal");
				return CURLE_OK;
			}
			else
			{
				remove(diskLocation.c_str());
			}
		}
		else
		{
			curl_trace("[downloadManager] success local file exist, but size and md5 empty");
			return CURLE_OK;
		}
	}

	localSize = GetLocalFileLenth(downLocation.c_str());

	if (localSize >= serverSize && serverSize != 0)
	{
		if (CheckMd5(serverMd5) == 0)
		{
			curl_trace("[downloadManager] success! Size and MD5 equal");
			return CURLE_OK;
		}
		else
		{
			if (localSize == serverSize) {
				curl_trace("[downloadManager] failed! Size equal but MD5 equal");
			} else {
				curl_trace("[downloadManager] failed! Download size bigger than real");
			}
			RemoveLocalFile(&stream, downLocation);
			return CURLE_READ_ERROR;
		}
	}

	stream = fopen(downLocation.c_str(), "a+b");
	if (!stream)  
	{  
		curl_trace("[downloadManager] failed! Open file err");
		return CURLE_WRITE_ERROR;  
	}  
	fseek(stream, 0, SEEK_END);

	CURLcode res;
	long Httpcode = 0;
	curlFile = curl_easy_init();  //初始化一个curl指针
	if(curlFile) { //curl对象存在的情况下执行的操作
		//设置远端地址
		curl_easy_setopt(curlFile, CURLOPT_URL, serverUrl.c_str());
		//执行写入文件流操作
		curl_easy_setopt(curlFile, CURLOPT_WRITEFUNCTION, &downLoadCallback);//当有数据被写入，回调函数被调用，
		curl_easy_setopt(curlFile, CURLOPT_WRITEDATA, this); //设置结构体的指针传递给回调函数
		// 设置重定向的最大次数  
		curl_easy_setopt(curlFile, CURLOPT_MAXREDIRS, 3);  
		// 设置301、302跳转跟随location  
		curl_easy_setopt(curlFile, CURLOPT_FOLLOWLOCATION, 1); 
		curl_easy_setopt(curlFile, CURLOPT_RESUME_FROM, (long)localSize);
		// 禁止SSL验证
		curl_easy_setopt(curlFile, CURLOPT_SSL_VERIFYPEER, 0);
		//curl_easy_setopt(curlFile, CURLOPT_NOPROGRESS, 0);  
		//设置进度回调函数  
		//curl_easy_setopt(curlFile, CURLOPT_PROGRESSFUNCTION, &ProgressCallback); 
		//写入文件
		res = curl_easy_perform(curlFile);
		if(res != CURLE_OK)
		{	
			if (cancelTask)
			{
				curl_trace("[downloadManager] task cancel");
				destoryresouce();
				return res;
			}

			curl_trace("[downloadManager] failed! Return value:%d",static_cast<int>(res));
			destoryresouce();
			return res;
		}

		CURLcode mycode = curl_easy_getinfo(curlFile, CURLINFO_RESPONSE_CODE, &Httpcode);
		if (Httpcode != 200 && Httpcode != 206 && Httpcode != 416)
		{
			curl_trace("[downloadManager] failed! Httpcode is:%d", static_cast<int>(Httpcode));
			RemoveLocalFile(&stream, downLocation);
			destoryresouce();
			return CURLE_COULDNT_RESOLVE_HOST;
		}
	}

	destoryresouce();
	if (CheckMd5(serverMd5))
	{
		RemoveLocalFile(&stream, downLocation);
		destoryresouce();
		curl_trace("[downloadManager] failed! Check file md5 error");
		return CURLE_WRITE_ERROR;
	}

	int ret = rename(downLocation.c_str(), diskLocation.c_str());
	if (ret != 0)
	{
		curl_trace("rename file:%s failed", downLocation.c_str());
	}

	curl_trace("[downloadManager] success!");
	return CURLE_OK;
}

int CurlDownManager::CheckMd5(const std::string& serverMD5)
{
	if (serverMD5.empty())
	{
		curl_trace("[downloadManager] MD5 empty!");
		return 0;
	}
	std::string md5 = BuildFileMd5(downLocation);

	//transform(md5.begin(), md5.end(), md5.begin(), ::toupper);小写转大写
	if (md5 == serverMD5)
	{
		curl_trace("[downloadManager] MD5 equal!");
		return 0;
	}

	curl_trace("Server md5 is:%s, Local md5 is:", serverMD5.c_str(), md5.c_str());
	return -1;
}

