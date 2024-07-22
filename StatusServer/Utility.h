#pragma once

///****************************************************************************
/// @Author      : haoks                                                       
/// @Date        : 
/// @Description : 工具类           
///****************************************************************************

#include <functional>

// 作为RAII管理类，实现回调函数的执行功能

class Defer
{
	using FUNC = std::function<void()>;
public:
	Defer(FUNC a) :m_func(a) {}
	~Defer() {
		m_func();
	}
private:
	FUNC m_func;
};

static std::string jsonSimpleStr(std::string json) {
	json.erase(std::remove_if(json.begin(), json.end(), [](unsigned char c) {
		return c == '\n' || c == '\r' || c == '\t';
		}), json.end());
	return json;
}

static std::string generate_unique_string() {
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);
	return unique_string;
}