#pragma once

// #include <unordered_map>
// #include <vector>

// struct CDownLoadTask;
// using TASKS = std::unordered_map<std::string, CDownLoadTask*>;

class Signleton
{
public:
	Signleton(const Signleton&) = delete;
	Signleton& operator = (const Signleton&) = delete;
	static Signleton* getInstance();

public:
//	TASKS tasks;

private:
	Signleton()
	{

	}

	~Signleton()
	{

	}
	static Signleton* instance;

	class free
	{
	public:
		~free()
		{
			if (instance != nullptr)
			{
				delete instance;
				instance = nullptr;
			}
		}
	};
	static free mf;
};

