#pragma once
#include "Singleton.hpp"
#include "const.h"
#include <map>
class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;

class LogicSystem :public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	bool HandleGet(std::string path, std::shared_ptr<HttpConnection> con);
	bool HandlePost(std::string path, std::shared_ptr<HttpConnection> con);
	void RegGet(std::string url, HttpHandler handler);
	void RegPost(std::string url, HttpHandler handler);
	~LogicSystem();
	 
private:
	LogicSystem();
	LogicSystem(const LogicSystem&) = delete;
	LogicSystem& operator=(const LogicSystem&) = delete;

private:
	std::map<std::string, HttpHandler> m_post_handlers;
	std::map<std::string, HttpHandler> m_get_handlers;
};

