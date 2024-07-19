#pragma once

///****************************************************************************
/// @Author      : haoks                                                       
/// @Date        : 
/// @Description : ������           
///****************************************************************************

#include <functional>

// ��ΪRAII�����࣬ʵ�ֻص�������ִ�й���
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


