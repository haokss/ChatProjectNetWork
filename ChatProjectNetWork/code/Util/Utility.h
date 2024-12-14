///****************************************************************************
/// @Author      : haoks                                                       
/// @Date        : 2024/12/14
/// @Description : ������           
///****************************************************************************
#pragma once
#include <functional>
#include <string>

// ��ΪRAII�����࣬ʵ�ֻص�������ִ�й���
//namespace ChatUtil
//{
	class Defer
	{
		using FUNC = std::function<void()>;
	public:
		Defer(FUNC a) 
			:m_func(a)
		{}
		~Defer() 
		{
			m_func();
		}
	private:
		FUNC m_func;
	};

	static std::string jsonSimpleStr(std::string json) 
	{
		json.erase(std::remove_if(json.begin(), json.end(), [](unsigned char c) 
		{
			return c == '\n' || c == '\r' || c == '\t';
		}), json.end());
		return json;
	}
//}
