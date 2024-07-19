#pragma once

///****************************************************************************
/// @Author      : haoks                                                       
/// @Date        : 
/// @Description : 工具类           
///****************************************************************************

#include <functional>

// 作为RAII管理类，实现回调函数的执行功能
using FUNC = std::function<void()>;
class Defer
{
public:
	Defer(FUNC a) :m_func(a) {}
	~Defer() {
		m_func();
	}
private:
	FUNC m_func;
};


