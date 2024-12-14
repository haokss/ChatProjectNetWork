#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include "CSession.h"
#include <map>
#include "../const.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class CServer:public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& ioc, unsigned short port);
	void Start();
	void hanldeAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& ec);
	void clearSession(string uuid);
private:
	tcp::acceptor m_acceptor;
	net::io_context& m_ioc;
	unsigned short m_port;
	std::map<std::string, std::shared_ptr<CSession>> m_sessions;
	std::mutex m_mtx;
};

