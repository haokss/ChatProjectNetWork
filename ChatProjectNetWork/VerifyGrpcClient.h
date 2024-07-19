#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.hpp"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPConPool {
public:
	RPConPool(size_t pool_size, std::string host, std::string port);	
	~RPConPool();
	void Close();
	std::unique_ptr<VarifyService::Stub> getConnection();
	void returnConnection(std::unique_ptr<VarifyService::Stub> context);
private:
	std::atomic<bool> m_stop;
	size_t m_pool_size;
	std::string m_host;
	std::string m_port;
	std::queue<std::unique_ptr<VarifyService::Stub>> m_que_connections;
	std::mutex m_mutex;
	std::condition_variable m_condition_var;
};


class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	~VerifyGrpcClient(){}
	GetVarifyRsp GetVrifyCode(std::string email);
private:
	VerifyGrpcClient();
private:
	//std::unique_ptr<VarifyService::Stub> p_stub;
	std::unique_ptr<RPConPool> p_pool; 
};

