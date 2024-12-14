#include "CServer.h"
#include <exception>
#include "AsioIOServicePool.h"
#include "UserMgr.h"
CServer::CServer(boost::asio::io_context& ioc, unsigned short port)
	:m_ioc(ioc), m_acceptor(ioc, tcp::endpoint(tcp::v4(), port)),m_port(port){
	std::cout << "server start success, listen on port" << m_port << std::endl;
	Start();
}

void CServer::Start(){
	auto& ioc_context = AsioIOServicePool::GetInstance().GetIOService();
	std::shared_ptr<CSession> new_con = std::make_shared<CSession>(ioc_context, this);
	m_acceptor.async_accept(new_con->GetSocket(), std::bind(&CServer::hanldeAccept, this,
		new_con, std::placeholders::_1));
}

void CServer::hanldeAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& ec){
	if (!ec) {
		new_session->Start();
		std::lock_guard<std::mutex> lock(m_mtx);
		// �˴����ݵ���int�����䲻ͬ
		m_sessions.insert(std::make_pair(new_session->GetSessionId(), new_session));
	}
	else {
		std::cout << "handleAccept()" << ec.what() << std::endl;
	}
	Start();
}

void CServer::clearSession(string uuid){
	auto it = m_sessions.find(uuid);
	if (it != m_sessions.end()) {
		// �Ƴ��û�����session
		UserMgr::GetInstance().removeUserSession(m_sessions[uuid]->GetUserId());
	}
	std::lock_guard<std::mutex> lock(m_mtx);
	m_sessions.erase(uuid);
}
