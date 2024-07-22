#pragma once
#include "const.h"
class HttpConnection :public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;
public:
	//HttpConnection(tcp::socket socket);
	HttpConnection(boost::asio::io_context& ioc);
	void Start();
	tcp::socket& GetSocket();
private:
	void CheckDeadLine();
	void WriteResponse();
	void HandleReq();
	void PreParseGetParam();
private:
	tcp::socket m_socket;
	beast::flat_buffer m_buffer{ 8192 };
	http::request<http::dynamic_body> m_request;
	http::response<http::dynamic_body> m_response;
	net::steady_timer m_deadline{
		m_socket.get_executor(), std::chrono::seconds(60)
	}; // …Ë÷√∂® ±∆˜
	std::string m_get_url;
	std::unordered_map<std::string, std::string> m_get_params;
};

