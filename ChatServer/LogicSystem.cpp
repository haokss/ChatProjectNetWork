#include "LogicSystem.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"
#include "const.h"
#include "RedisMgr.h"
#include "UserMgr.h"
using namespace std;

LogicSystem::LogicSystem():_b_stop(false){
	RegisterCallBacks();
	_worker_thread = std::thread (&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem(){
	_b_stop = true;
	_consume.notify_one();
	_worker_thread.join();
}

void LogicSystem::PostMsgToQue(shared_ptr < LogicNode> msg) {
	std::unique_lock<std::mutex> unique_lk(_mutex);
	_msg_que.push(msg);
	//由0变为1则发送通知信号
	if (_msg_que.size() == 1) {
		unique_lk.unlock();
		_consume.notify_one();
	}
}

void LogicSystem::DealMsg() {
	for (;;) {
		std::unique_lock<std::mutex> unique_lk(_mutex);
		//判断队列为空则用条件变量阻塞等待，并释放锁
		while (_msg_que.empty() && !_b_stop) {
			_consume.wait(unique_lk);
		}

		//判断是否为关闭状态，把所有逻辑执行完后则退出循环
		if (_b_stop ) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
				auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
				if (call_back_iter == _fun_callbacks.end()) {
					_msg_que.pop();
					continue;
				}
				call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
				_msg_que.pop();
			}
			break;
		}

		//如果没有停服，且说明队列中有数据
		auto msg_node = _msg_que.front();
		cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
		auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
		if (call_back_iter == _fun_callbacks.end()) {
			_msg_que.pop();
			std::cout << "msg id [" << msg_node->_recvnode->_msg_id << "] handler not found" << std::endl;
			continue;
		}
		call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id, 
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
		_msg_que.pop();
	}
}

void LogicSystem::RegisterCallBacks() {
	_fun_callbacks[MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3);	
}

void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short &msg_id, const string &msg_data) {
	Json::Reader reader;
	Json::Value root;
	bool parsingSuccessful = reader.parse(msg_data, root);
	std::cout << msg_data << std::endl;
	if (!parsingSuccessful) {
		std::cerr << "Failed to parse JSON: " << reader.getFormattedErrorMessages() << std::endl;
		return;
	}
	if (!root.isMember("uid") || !root.isMember("token")) {
		std::cerr << "JSON does not contain required fields 'uid' and 'token'" << std::endl;
		return;
	}
	Json::Value  rtvalue;
	// 函数调用结束自动回包
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, MSG_CHAT_LOGIN_RSP);
		});
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "user login uid is  " << uid << " user token  is "
		<< token << endl;
	// 不去status服务器中查询token，直接在redis中查找status服务器设置的token
	//auto rsp = StatusGrpcClient::GetInstance().Login(uid, token);
	//rtvalue["error"] = rsp.error();
	//auto value = rtvalue["error"];
	//if (rsp.error() != ErrorCodes::Success) {
	//	std::string return_str = rtvalue.toStyledString();
	//	session->Send(return_str, MSG_CHAT_LOGIN_RSP);
	//	return;
	//}
	std::string token_key = USERTOKENPREFIX + std::to_string(uid);
	std::string token_val = "";
	bool success = RedisMgr::GetInstance().Get(token_key, token_val);
	if (!success) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	if (token_val != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
	}
	rtvalue["error"] = ErrorCodes::Success;
	std::shared_ptr<UserInfo> user_info = nullptr;	
	user_info = MysqlMgr::GetInstance().GetUser(uid);
	rtvalue["uid"] = uid;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;

	// 将服务器上线人数增加
	std::string server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
	auto rd_res = RedisMgr::GetInstance().HGet(LOGIN_COUNT, server_name);
	int login_count = 0;
	if (!rd_res.empty()) {
		login_count = std::stoi(rd_res);
	}
	++login_count;
	RedisMgr::GetInstance().HSet(LOGIN_COUNT, server_name, std::to_string(login_count));
	// 为session绑定id,方便后期管理
	session->SetUserId(uid);
	std::string ip_key = USERIPPREFIX + std::to_string(uid);
	RedisMgr::GetInstance().Set(ip_key, server_name);
	UserMgr::GetInstance().setUserSession(uid, session);
	return;
}



bool LogicSystem::isPureDigit(const std::string& str)
{
	for (char c : str) {
		if (!std::isdigit(c)) {
			return false;
		}
	}
	return true;
}
