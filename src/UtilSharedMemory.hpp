//
// Created By: Huiyong.Men 2019/07/09
//
#pragma once

#include <string>

#ifdef WIN32
#include <windows.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#endif

struct SharedMemoryHandle
{
	std::string label;
	int size;
#ifdef WIN32
	HANDLE handle;
	HANDLE mutexHandle;
	LPVOID lp;
#else
    int fd;
	void *p;
#endif
};

class SharedMemory
{
public:
	static bool sharedMemoryCreateImpl(std::string label, int size, SharedMemoryHandle& handle);
	static void sharedMemoryDestroyImpl(SharedMemoryHandle& handle);
	static bool sharedMemoryWriteImpl(std::string label, int size, const char* data);
	static bool sharedMemoryReadImpl(std::string label, int size, char* data);
	static bool sharedMemoryExistImpl(std::string label);
	static bool sharedMemoryOpenMapImpl(std::string label, int size, SharedMemoryHandle& handle);
	static bool sharedMemoryCloseMapImpl(SharedMemoryHandle& handle,int size);
};

#define SharedMemoryCreate SharedMemory::sharedMemoryCreateImpl
#define SharedMemoryDestroy SharedMemory::sharedMemoryDestroyImpl
#define SharedMemoryWrite SharedMemory::sharedMemoryWriteImpl
#define SharedMemoryRead SharedMemory::sharedMemoryReadImpl
#define SharedMemoryExist SharedMemory::sharedMemoryExistImpl
#define SharedMemoryOpenMap SharedMemory::sharedMemoryOpenMapImpl
#define SharedMemoryCloseMap SharedMemory::sharedMemoryCloseMapImpl

