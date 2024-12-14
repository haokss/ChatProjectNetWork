///****************************************************************************
/// @Author      : haoks                                                       
/// @Date        : 2024/12/14
/// @Description : 工具类           
///****************************************************************************
#pragma once
#include <functional>
#include <string>

// 作为RAII管理类，实现回调函数的执行功能
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
