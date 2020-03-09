#pragma once
#include <string>

// ��ȡ�����������
#define CURL_TASK_START_DATA "CURL_TASK_START_DATA"
// ��������,���ȵ��������ڴ�
#define CURL_TASK_POST_DATA "CURL_TASK_POST_DATA"


struct FtpFile {
	FtpFile() :stream(nullptr) {}
	std::string downLocation;														//��������λ��
	FILE * stream;																	//�ļ���
	std::string serverUrl;															//����urlԴ
	std::string serverName;															//�ļ���(����ʾ�����ؽ�����)
	std::string serverMd5;															//�ļ�MD5
	int64_t serverSize;																//�ļ���С
	
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
	char taskDatas[1024 * 2];	// json����
};

struct TaskCallBackData 
{
	char taskCallbackDatas[1024];	// json����
};
