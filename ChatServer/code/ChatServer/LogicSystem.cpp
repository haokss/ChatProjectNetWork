#include "LogicSystem.h"
#include "../grpc/StatusGrpcClient.h"
#include "../MySqlConnect/MysqlMgr.h"
#include "../const.h"
#include "../RedisManager/RedisMgr.h"
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
	_fun_callbacks[ID_SEARCH_USER_REQ] = std::bind(&LogicSystem::SearchInfo, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
}

void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short &msg_id, const string &msg_data) {
	Json::Reader reader;
	Json::Value root;
	bool parsingSuccessful = reader.parse(msg_data, root);
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

}

void LogicSystem::SearchInfo(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data){
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	std::string uid_str = root["uid"].asString();
	Json::Value rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_SEARCH_USER_RSP);
		});
	if (isPureDigit(uid_str)) {
		GetUserByUid(uid_str, rtvalue);
	}
}

bool LogicSystem::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMgr::GetInstance().Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		userinfo->uid = root["uid"].asInt();
		userinfo->name = root["name"].asString();
		userinfo->pwd = root["pwd"].asString();
		userinfo->email = root["email"].asString();
		userinfo->nick = root["nick"].asString();
		userinfo->desc = root["desc"].asString();
		userinfo->sex = root["sex"].asInt();
		userinfo->icon = root["icon"].asString();
		std::cout << "user login uid is  " << userinfo->uid << " name  is "
			<< userinfo->name << " pwd is " << userinfo->pwd << " email is " << userinfo->email << endl;
	}
	else {
		//redis中没有则查询mysql
		//查询数据库
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = MysqlMgr::GetInstance().GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}
		userinfo = user_info;
		//将数据库内容写入redis缓存
		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pwd"] = userinfo->pwd;
		redis_root["name"] = userinfo->name;
		redis_root["email"] = userinfo->email;
		redis_root["nick"] = userinfo->nick;
		redis_root["desc"] = userinfo->desc;
		redis_root["sex"] = userinfo->sex;
		redis_root["icon"] = userinfo->icon;
		RedisMgr::GetInstance().Set(base_key, redis_root.toStyledString());
	}

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

void LogicSystem::GetUserByUid(std::string uid_str, Json::Value& rtvalue){
	rtvalue["error"] = ErrorCodes::Success;
	std::string base_key = USER_BASE_INFO + uid_str;
	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMgr::GetInstance().Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();
		std::cout << "user  uid is  " << uid << " name  is "
			<< name << " pwd is " << pwd << " email is " << email << " icon is " << icon << endl;
		rtvalue["uid"] = uid;
		rtvalue["pwd"] = pwd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		rtvalue["icon"] = icon;
		return;
	}

	auto uid = std::stoi(uid_str);
	//redis中没有则查询mysql
	//查询数据库
	std::shared_ptr<UserInfo> user_info = nullptr;
	user_info = MysqlMgr::GetInstance().GetUser(uid);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	//将数据库内容写入redis缓存
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->pwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	RedisMgr::GetInstance().Set(base_key, redis_root.toStyledString());
	auto server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
	//将登录数量增加
	auto rd_res = RedisMgr::GetInstance().HGet(LOGIN_COUNT, server_name);
	int count = 0;
	if (!rd_res.empty()) {
		count = std::stoi(rd_res);
	}
	count++;
	auto count_str = std::to_string(count);
	RedisMgr::GetInstance().HSet(LOGIN_COUNT, server_name, count_str);
	//返回数据
	rtvalue["uid"] = user_info->uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;
}

//void LogicSystem::GetUserByName(std::string name, Json::Value& rtvalue){
//	rtvalue["error"] = ErrorCodes::Success;
//	std::string base_key = NAME_INFO + name;
//	//优先查redis中查询用户信息
//	std::string info_str = "";
//	bool b_base = RedisMgr::GetInstance().Get(base_key, info_str);
//	if (b_base) {
//		Json::Reader reader;
//		Json::Value root;
//		reader.parse(info_str, root);
//		auto uid = root["uid"].asInt();
//		auto name = root["name"].asString();
//		auto pwd = root["pwd"].asString();
//		auto email = root["email"].asString();
//		auto nick = root["nick"].asString();
//		auto desc = root["desc"].asString();
//		auto sex = root["sex"].asInt();
//		std::cout << "user  uid is  " << uid << " name  is "
//			<< name << " pwd is " << pwd << " email is " << email << endl;
//		rtvalue["uid"] = uid;
//		rtvalue["pwd"] = pwd;
//		rtvalue["name"] = name;
//		rtvalue["email"] = email;
//		rtvalue["nick"] = nick;
//		rtvalue["desc"] = desc;
//		rtvalue["sex"] = sex;
//		return;
//	}
//	//redis中没有则查询mysql
//	//查询数据库
//	std::shared_ptr<UserInfo> user_info = nullptr;
//	user_info = MysqlMgr::GetInstance().GetUser(name);
//	if (user_info == nullptr) {
//		rtvalue["error"] = ErrorCodes::UidInvalid;
//		return;
//	}
//
//	//将数据库内容写入redis缓存
//	Json::Value redis_root;
//	redis_root["uid"] = user_info->uid;
//	redis_root["pwd"] = user_info->pwd;
//	redis_root["name"] = user_info->name;
//	redis_root["email"] = user_info->email;
//	redis_root["nick"] = user_info->nick;
//	redis_root["desc"] = user_info->desc;
//	redis_root["sex"] = user_info->sex;
//
//	RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
//	auto server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
//	//将登录数量增加
//	auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
//	int count = 0;
//	if (!rd_res.empty()) {
//		count = std::stoi(rd_res);
//	}
//
//	count++;
//	auto count_str = std::to_string(count);
//	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);
//
//	//返回数据
//	rtvalue["uid"] = user_info->uid;
//	rtvalue["pwd"] = user_info->pwd;
//	rtvalue["name"] = user_info->name;
//	rtvalue["email"] = user_info->email;
//	rtvalue["nick"] = user_info->nick;
//	rtvalue["desc"] = user_info->desc;
//	rtvalue["sex"] = user_info->sex;
//}
