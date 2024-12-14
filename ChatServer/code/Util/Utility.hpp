#ifndef _UTILITY_HPP
#define _UTILITY_HPP

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
#endif