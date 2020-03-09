#include "UtilSharedMemory.hpp"
#include <string>
#include <codecvt>
#include <iostream>

#define SM_Mutex_TAIL_FIX "_SM_Mutex"

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
#ifdef WIN32
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
#endif

bool SharedMemory::sharedMemoryCreateImpl(std::string label, int size, SharedMemoryHandle& handle)
{
    handle.label = label;
    handle.size = size;

#ifdef WIN32
    HANDLE hMutex = CreateMutex(NULL, FALSE, u2w(std::string(label).append(SM_Mutex_TAIL_FIX)).c_str());
    if (!hMutex) {
        return false;
    }

    handle.handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, u2w(label).c_str());
    if (!handle.handle) {
        return false;
    }

    handle.lp = MapViewOfFile(handle.handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!handle.lp) {
        CloseHandle(handle.handle);
        return false;
    }
#else

    sem_t* mutex = sem_open(std::string(label).c_str(), O_CREAT, 0644, 1);
    if(mutex == SEM_FAILED) {
        return false;
    }
    handle.fd = shm_open(label.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (handle.fd < 0) {
        return false;
    }

    #endif
    return true;
}

void SharedMemory::sharedMemoryDestroyImpl(SharedMemoryHandle& handle)
{
#ifdef WIN32
    UnmapViewOfFile(handle.lp);
    CloseHandle(handle.handle);
#else
    close(handle.fd);
    shm_unlink(handle.label.c_str());
    munmap(handle.p, handle.size);
    sem_unlink(handle.label.c_str());
#endif
}

bool SharedMemory::sharedMemoryWriteImpl(std::string label, int size, const char* data)
{
#ifdef WIN32
    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, u2w(std::string(label).append(SM_Mutex_TAIL_FIX)).c_str());
    if (!hMutex) {
        return false;
    }

    WaitForSingleObject(hMutex, INFINITE);
    HANDLE handle = OpenFileMapping(FILE_MAP_WRITE, NULL, u2w(label).c_str());
    if (!handle) {
        return false;
    }

    LPVOID lp = MapViewOfFile(handle, FILE_MAP_WRITE, 0, 0, 0);
    memcpy(lp, data, size);
    UnmapViewOfFile(lp);
    CloseHandle(handle);
    ReleaseMutex(hMutex);
#else

    sem_t* mutex  = sem_open(std::string(label).c_str(), O_CREAT, 0644, 1);

    if(mutex == SEM_FAILED) {
        return false;
    }

    sem_wait(mutex);
    int fd = shm_open(label.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        sem_post(mutex);
        sem_close(mutex);
        return false;
    }
    
    if(ftruncate(fd, size) == -1) {
        sem_post(mutex);
        sem_close(mutex);
        return false;
    }
    
    void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(p == MAP_FAILED) {
        sem_post(mutex);
        sem_close(mutex);
        return false;
    }
    
    memcpy(p, data, size);
    sem_post(mutex);
    sem_close(mutex);
#endif
    return true;
}

bool SharedMemory::sharedMemoryReadImpl(std::string label, int size, char* data)
{
#ifdef WIN32
    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, u2w(std::string(label).append(SM_Mutex_TAIL_FIX)).c_str());
    if (!hMutex) {
        return false;
    }

    WaitForSingleObject(hMutex, INFINITE);
    HANDLE handle = OpenFileMapping(FILE_MAP_READ, NULL, u2w(label).c_str());
    if (!handle) {
        return false;
    }

    LPVOID lp = MapViewOfFile(handle, FILE_MAP_READ, 0, 0, 0);
    memcpy(data, lp, size);
    UnmapViewOfFile(lp);
    CloseHandle(handle);
    ReleaseMutex(hMutex);
#else

    sem_t* mutex  = sem_open(std::string(label).c_str(), O_CREAT, 0644, 1);
    if(mutex == SEM_FAILED) {
        return false;
    }

    sem_wait(mutex);
    int fd = shm_open(label.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        sem_post(mutex);
        sem_close(mutex);
        return false;
    }

    if(ftruncate(fd, size) == -1) {
        sem_post(mutex);
        sem_close(mutex);
        return false;
    }

    void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(p == MAP_FAILED) {
        sem_post(mutex);
        sem_close(mutex);
        return false;
    }

    memcpy(data, p, size);
    sem_post(mutex);
    sem_close(mutex);
#endif
    return true;
}

bool SharedMemory::sharedMemoryExistImpl(std::string label)
{
#ifdef WIN32
    HANDLE handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, u2w(label).c_str());
    if (!handle) {
        return false;
    }

    CloseHandle(handle);
#else
 
    int fd = shm_open(label.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return false;
    }
#endif
    return true;
}

bool SharedMemory::sharedMemoryOpenMapImpl(std::string label, int size, SharedMemoryHandle & handle)
{
#ifdef WIN32
    handle.label = label;
    handle.mutexHandle = OpenMutex(MUTEX_ALL_ACCESS, FALSE, u2w(std::string(label).append(SM_Mutex_TAIL_FIX)).c_str());
    if (!handle.mutexHandle) {
        return false;
    }

    WaitForSingleObject(handle.mutexHandle, INFINITE);
    handle.handle = OpenFileMapping(FILE_MAP_WRITE, NULL, u2w(label).c_str());
    if (!handle.handle) {
        return false;
    }

    handle.lp = MapViewOfFile(handle.handle, FILE_MAP_WRITE, 0, 0, 0);
#else

    handle.label = label;
    sem_t* mutex  = sem_open(std::string(label).c_str(), O_CREAT, 0644, 1);
    if(mutex == SEM_FAILED) {
        return false;
    }

    sem_wait(mutex);
    int fd = shm_open(label.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        sem_post(mutex);
        sem_close(mutex);
        return false;
    }

    handle.p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(handle.p == MAP_FAILED) {
        sem_post(mutex);
        sem_close(mutex);
        return false;
    }

    sem_post(mutex);
    sem_close(mutex);
#endif
    return true;
}

bool SharedMemory::sharedMemoryCloseMapImpl(SharedMemoryHandle & handle,int size)
{
#ifdef WIN32
    UnmapViewOfFile(handle.lp);
    CloseHandle(handle.handle);
    ReleaseMutex(handle.mutexHandle);
#else
    munmap(handle.p,size);
    close(handle.fd);
#endif
    return true;
}
