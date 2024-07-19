#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"
// class RPConPool
RPConPool::RPConPool(size_t pool_size, std::string host, std::string port)
	:m_pool_size(pool_size), m_host(host), m_port(port), m_stop(false) {
	for (size_t i = 0; i < pool_size; ++i) {
		std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
			grpc::InsecureChannelCredentials());
		auto m = VarifyService::NewStub(channel);
		m_que_connections.push(std::move(m));
	}
}

RPConPool::~RPConPool()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	Close();
	while (!m_que_connections.empty()) {
		m_que_connections.pop();
	}
}

void RPConPool::Close(){
	m_stop = true;
	m_condition_var.notify_all();
}

std::unique_ptr<VarifyService::Stub> RPConPool::getConnection()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	// 等待队列不为空，或者停止
	m_condition_var.wait(lock, [this] {
		return m_stop || !m_que_connections.empty();
		});
	if (m_stop) {
		return nullptr;
	}
	auto context = std::move(m_que_connections.front());
	m_que_connections.pop();
	return context;
}

void RPConPool::returnConnection(std::unique_ptr<VarifyService::Stub> context){
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_stop) {
		return;
	}
	m_que_connections.push(std::move(context));
	m_condition_var.notify_one();
}

// class VerifyGrpcClient
GetVarifyRsp VerifyGrpcClient::GetVrifyCode(std::string email) {
	ClientContext context;
	GetVarifyRsp reply;
	GetVarifyReq request;
	request.set_email(email);
	//Status status = p_stub->GetVarifyCode(&context, request, &reply);
	auto stub = p_pool->getConnection();
	Status status = stub->GetVarifyCode(&context, request, &reply);
	if (status.ok()) {
		p_pool->returnConnection(std::move(stub));
		return reply;
	}
	else {
		p_pool->returnConnection(std::move(stub));
		reply.set_error(ErrorCodes::RPCFAILED);
		return reply;
	}
}

VerifyGrpcClient::VerifyGrpcClient() {
	auto& gCfgMgr = ConfigMgr::Inst();
	std::string host = gCfgMgr["VerifyServer"]["Host"];
	std::string port = gCfgMgr["VerifyServer"]["Port"];
	//std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051",
		//grpc::InsecureChannelCredentials());
	p_pool.reset(new RPConPool(5, host, port));

	//p_stub = VarifyService::NewStub(channel);
}
