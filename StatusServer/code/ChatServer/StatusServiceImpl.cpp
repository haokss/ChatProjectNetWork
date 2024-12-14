#include "StatusServiceImpl.h"
#include "../ConfigManager/ConfigMgr.h"
#include "../const.h"
#include "../RedisManager/RedisMgr.h"
#include "../Util/Utility.hpp"
#include <climits>

std::string generate_unique_string()
{
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();

	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);

	return unique_string;
}

StatusServiceImpl::StatusServiceImpl()
{
	auto& cfg = ConfigMgr::Inst();
	ChatServer server;
	server.port = cfg["ChatServer1"]["Port"];
	server.host = cfg["ChatServer1"]["Host"];
	server.name = cfg["ChatServer1"]["Name"];
	server.con_count = 0;
	_servers[server.name] = server;
	server.port = cfg["ChatServer2"]["Port"];
	server.host = cfg["ChatServer2"]["Host"];
	server.name = cfg["ChatServer2"]["Name"];
	_servers[server.name] = server;
}


Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
    std::string prefix("haoks status server has received : ");
    //auto& server = _servers[_server_index];
	const auto& server = getChatServer();
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::SUCCESS);
    reply->set_token(generate_unique_string());
	insertToken(request->uid(), reply->token());
    return Status::OK;
}


//Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
//{
//	std::string prefix("haoks status server has received :  ");
//	const auto& server = getChatServer();
//	reply->set_host(server.host);
//	reply->set_port(server.port);
//	reply->set_error(ErrorCodes::SUCCESS);
//	reply->set_token(generate_unique_string());
//	insertToken(request->uid(), reply->token());
//	return Status::OK;
//}
//
//StatusServiceImpl::StatusServiceImpl()
//{
//	auto& cfg = ConfigMgr::Inst();
//	ChatServer server;
//	server.port = cfg["ChatServer1"]["Port"];
//	server.host = cfg["ChatServer1"]["Host"];
//	server.con_count = 0;
//	server.name = cfg["ChatServer1"]["Name"];
//	_servers[server.name] = server;
//
//	server.port = cfg["ChatServer2"]["Port"];
//	server.host = cfg["ChatServer2"]["Host"];
//	server.con_count = 0;
//	server.name = cfg["ChatServer2"]["Name"];
//	_servers[server.name] = server;
//
//}
//
ChatServer StatusServiceImpl::getChatServer() {
	std::lock_guard<std::mutex> guard(_server_mtx);
	auto minServer = _servers.begin()->second;
	auto count_str = RedisMgr::GetInstance().HGet(LOGIN_COUNT, minServer.name);
	if (count_str.empty()) {
		//不存在则默认设置为最大
		minServer.con_count = INT_MAX;
	}
	else {
		minServer.con_count = std::stoi(count_str);
	}
	// 使用范围基于for循环
	for (auto& server : _servers) {
		if (server.second.name == minServer.name) {
			continue;
		}
		auto count_str = RedisMgr::GetInstance().HGet(LOGIN_COUNT, server.second.name);
		if (count_str.empty()) {
			server.second.con_count = INT_MAX;
			std::cout << server.first << " is not exist" << std::endl;
		}
		else {
			server.second.con_count = std::stoi(count_str);
		}
		if (server.second.con_count < minServer.con_count) {
			minServer = server.second;
		}
	}
	return minServer;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
	auto uid = request->uid();
	auto token = request->token();
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisMgr::GetInstance().Get(token_key, token_value);
	if (!success) {
		reply->set_error(ErrorCodes::ERROR_UID_INVALID);
		return Status::OK;
	}
	if (token_value != token) {
		reply->set_error(ErrorCodes::ERROR_TOKEN_INVALID);
		return Status::OK;
	}
	reply->set_error(ErrorCodes::SUCCESS);
	reply->set_uid(uid);
	reply->set_token(token);
	return Status::OK;
}

void StatusServiceImpl::insertToken(int uid, std::string token)
{
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	RedisMgr::GetInstance().Set(token_key, token);
	std::lock_guard<std::mutex> lock(_token_mtx);
	_tokens[uid] = token;
}

