#pragma once
#include <io.h>
#include <direct.h>
#include <sstream>
#include "MD5.h"
#include <codecvt>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#pragma warning (disable:4996)

// FileMD5
static std::string BuildFileMd5(const std::string &fileName)
{
	std::ifstream in(fileName.c_str(), std::ios::binary);
	if (!in)
		return "";

	MD5 md5;
	std::streamsize length;
	char buffer[1024];
	while (!in.eof()) {
		in.read(buffer, 1024);
		length = in.gcount();
		if (length > 0)
			md5.update(buffer, length);
	}
	in.close();
	return md5.toString();
}

// 获得已下载的文件大小
static long GetLocalFileLenth(const std::string &fileName)
{
	char strTemp[256] = { 0 };
	strcpy_s(strTemp, fileName.c_str());
	FILE* fp = fopen(strTemp, "rb");
	if (fp != NULL)
	{
		long localSizeTmp = _filelength(_fileno(fp));
		int closevalue = fclose(fp);
		return localSizeTmp;
	}
	return 0;
}

static void RemoveLocalFile(FILE** local, const std::string& localName)
{
	if (local)
	{
		fflush(*local);
		//关闭文件流
		int fclosecode = fclose(*local);
		if (fclosecode == 0)
		{
			*local = nullptr;
		}
	}
	remove(localName.c_str());
}

static std::string getExePath()
{
	char szFilePath[MAX_PATH + 1] = { 0 };
	::GetModuleFileNameA(NULL, szFilePath, MAX_PATH);

	std::string exePath(szFilePath);
	std::size_t pos = exePath.rfind('\\');
	if (pos != std::string::npos)
	{
		return exePath.substr(0, pos + 1);
	}
	return "";
}

static int32_t CreateDirectory(const std::string& folderPath)
{
	auto dirPathLen = folderPath.length();
	char tmpDirPath[256] = { 0 };
	for (auto i = 0; i < dirPathLen; ++i)
	{
		tmpDirPath[i] = folderPath[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (0 != access(tmpDirPath, 0))
			{
				int32_t ret = mkdir(tmpDirPath);
				if (ret != 0)
				{
					return ret;
				}
			}
		}
	}	
	return 0;
}

static bool FileExist(const std::string sPath, const bool bFloder)
{
	if (bFloder)
	{
		return (PathIsDirectoryA(sPath.c_str()) == FILE_ATTRIBUTE_DIRECTORY);
	}
	else
	{
		std::ifstream fFile(sPath);
		if (fFile)
		{
			fFile.close();
			return true;
		}
	}
	return false;
}

static std::string W2A_UTF8(const std::wstring& strIn)
{
	DWORD dwNum = WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), -1, NULL, 0, NULL, NULL);
	std::string sTmp(dwNum, 0);
	WideCharToMultiByte(CP_UTF8, 0, strIn.c_str(), -1, &sTmp[0], dwNum, NULL, NULL);
	return sTmp;
}

static std::wstring A2W_UTF8(const std::string& strIn)
{
	DWORD dwNum = MultiByteToWideChar(CP_UTF8, 0, strIn.c_str(), -1, NULL, 0);
	std::wstring sTmp(dwNum, 0);
	MultiByteToWideChar(CP_UTF8, 0, strIn.c_str(), -1, &sTmp[0], dwNum);
	return sTmp;
}

static std::string W2A_ACP(const std::wstring& strIn)
{
	DWORD dwNum = WideCharToMultiByte(CP_ACP, 0, strIn.c_str(), -1, NULL, 0, NULL, NULL);
	std::string sTmp(dwNum, 0);
	WideCharToMultiByte(CP_ACP, 0, strIn.c_str(), -1, &sTmp[0], dwNum, NULL, NULL);
	return sTmp;
}

static std::wstring A2W_ACP(const std::string& strIn)
{
	DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, strIn.c_str(), -1, NULL, 0);
	std::wstring sTmp(dwNum, 0);
	MultiByteToWideChar(CP_ACP, 0, strIn.c_str(), -1, &sTmp[0], dwNum);
	return sTmp;
}


#include <time.h>
#include <stdarg.h>
static char g_log_file[MAX_PATH] = { 0 };
static void curl_trace(const char *fmt, ...)
{
	const char * mode = "a+";
	if (strlen(g_log_file) == 0)
	{
		mode = "a+";
		_snprintf(g_log_file, MAX_PATH, (getExePath() + "CurlDownloadManager.log").c_str());
	}
	FILE *log_fp;
	if ((log_fp = fopen(g_log_file, mode)) == NULL) {
		perror("open vpn_log.log err");
		return;
	}
	va_list args;
	va_start(args, fmt);
	time_t now;
	time(&now);
	char ch_time[30];
	struct tm* time;
	time = localtime(&now);
	sprintf(ch_time, "%04d-%02d-%02d %02d:%02d:%02d", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
	fprintf(log_fp, "%s: ", ch_time);
	vfprintf(log_fp, fmt, args);
	fprintf(log_fp, "\r\n");
	va_end(args);
	fflush(log_fp);
	fclose(log_fp);
}