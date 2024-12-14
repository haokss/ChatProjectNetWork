#include "CServer.h"
#include <exception>
#include "HttpConnection.h"
#include "AsioIOServicePool.h"
CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
	:m_ioc(ioc), m_acceptor(ioc, tcp::endpoint(tcp::v4(), port)){

}

void CServer::Start(){
	auto self = shared_from_this();
	auto& ioc_context = AsioIOServicePool::GetInstance().GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(ioc_context);
	m_acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
		try {
			if (ec) {
				self->Start();
				return;
			}
			// 创建新连接，管理连接
			//std::make_shared<HttpConnection>(std::move(self->m_socket))->Start();
			new_con->Start();
			self->Start();
		}
		catch (std::exception& e) {
			log4cpp::Category::getInstance("server").error(e.what());
		}
		});
}
