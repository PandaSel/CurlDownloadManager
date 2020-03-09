//
// Created By: Huiyong.Men 2019/07/09
//
#pragma once

#include <string>

#include <windows.h>

struct SharedMemoryHandle
{
	std::string label;
	int size;
	HANDLE handle;
	LPVOID lp;
};

class SharedMemory
{
public:
	static bool sharedMemoryCreateImpl(std::string label, int size, SharedMemoryHandle& handle);
	static void sharedMemoryDestroyImpl(SharedMemoryHandle& handle);
	static bool sharedMemoryWriteImpl(std::string label, int size, const char* data);
	static bool sharedMemoryReadImpl(std::string label, int size, char* data);
	static bool sharedMemoryExistImpl(std::string label);
};

#define SharedMemoryCreate SharedMemory::sharedMemoryCreateImpl
#define SharedMemoryDestroy SharedMemory::sharedMemoryDestroyImpl
#define SharedMemoryWrite SharedMemory::sharedMemoryWriteImpl
#define SharedMemoryRead SharedMemory::sharedMemoryReadImpl
#define SharedMemoryExist SharedMemory::sharedMemoryExistImpl

