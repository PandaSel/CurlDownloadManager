#include "Singleton.h"
#include<mutex>

Signleton* Signleton::instance = nullptr;
Signleton::free Signleton::mf;
std::mutex mtx;

Signleton* Signleton::getInstance()
{
	if (instance == nullptr)
	{
		std::lock_guard<std::mutex> guard(mtx);
		if (instance == nullptr)
		{
			instance = new Signleton;
		}
	}
	return instance;
}
