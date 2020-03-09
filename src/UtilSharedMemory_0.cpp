#include "UtilSharedMemory.hpp"
#include <string>
#include <codecvt>

class SharedMemorySender
{
public:
	SharedMemorySender(const std::string& label, int size);
	~SharedMemorySender();

	bool Send(const char* data);

private:
	SharedMemoryHandle mMapFile;
};

class SharedMemoryReceiver
{
public:
	SharedMemoryReceiver(const std::string& label, int size);
	~SharedMemoryReceiver();

	bool Receive(char* data);

private:
	void OpenFileMap();

	SharedMemoryHandle mMapFile;
};

std::wstring u2w(const std::string &us)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
	return conv.from_bytes(us);
}

SharedMemoryReceiver::SharedMemoryReceiver(const std::string& label, int size)
{
    mMapFile.label = label;
    mMapFile.size = size;
    OpenFileMap();
}

SharedMemoryReceiver::~SharedMemoryReceiver()
{
    if (mMapFile.handle) {
        UnmapViewOfFile(mMapFile.lp);
        CloseHandle(mMapFile.handle);
    }
}

void SharedMemoryReceiver::OpenFileMap()
{
    mMapFile.handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, (LPCSTR)(mMapFile.label.c_str()));
    if (mMapFile.handle) {
        mMapFile.lp = MapViewOfFile(mMapFile.handle, FILE_MAP_ALL_ACCESS, 0, 0, mMapFile.size);
        if (!mMapFile.lp)
        {
            CloseHandle(mMapFile.handle);
            mMapFile.handle = nullptr;
        }
    }
}

bool SharedMemoryReceiver::Receive(char* data)
{
    if (!mMapFile.handle) {
        OpenFileMap();
    }

    if (mMapFile.handle) {
        memcpy(data, mMapFile.lp, mMapFile.size);
        return true;
    }

    return false;
}

SharedMemorySender::SharedMemorySender(const std::string& label, int size)
{
    mMapFile.handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, (LPCSTR)(label.c_str()));
    if (mMapFile.handle) {
        mMapFile.lp = MapViewOfFile(mMapFile.handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
        if (!mMapFile.lp)
        {
            CloseHandle(mMapFile.handle);
            mMapFile.handle = nullptr;
        }
    }
    mMapFile.size = size;
}

SharedMemorySender::~SharedMemorySender()
{
    if (mMapFile.handle) {
        UnmapViewOfFile(mMapFile.lp);
        CloseHandle(mMapFile.handle);
    }
}

bool SharedMemorySender::Send(const char* data)
{
    if (mMapFile.handle) {
        memcpy(mMapFile.lp, data, mMapFile.size);
        return true;
    }

    return false;
}

bool SharedMemory::sharedMemoryCreateImpl(std::string label, int size, SharedMemoryHandle& handle)
{
	handle.label = label;
	handle.size = size;
	handle.handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, u2w(label).c_str());
	if (!handle.handle)
		return false;

	handle.lp = MapViewOfFile(handle.handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
	if (!handle.lp)
	{
		CloseHandle(handle.handle);
		return false;
	}
	return true;
}

void SharedMemory::sharedMemoryDestroyImpl(SharedMemoryHandle& handle)
{
	UnmapViewOfFile(handle.lp);
	CloseHandle(handle.handle);
}

bool SharedMemory::sharedMemoryWriteImpl(std::string label, int size, const char* data)
{
	HANDLE handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, u2w(label).c_str());
	if (!handle)
		return false;
	LPVOID lp = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	memcpy(lp, data, size);
	UnmapViewOfFile(lp);
	CloseHandle(handle);
	return true;
}

bool SharedMemory::sharedMemoryReadImpl(std::string label, int size, char* data)
{
	HANDLE handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, u2w(label).c_str());
	if (!handle)
		return false;
	LPVOID lp = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	memcpy(data, lp, size);
	UnmapViewOfFile(lp);
	CloseHandle(handle);
	return true;
}

bool SharedMemory::sharedMemoryExistImpl(std::string label)
{
	HANDLE handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, u2w(label).c_str());
	if (!handle)
		return false;
	CloseHandle(handle);
	return true;
}