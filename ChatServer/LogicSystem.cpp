#include "LogicSystem.h"
#include "StatusGrpcClient.h"
#include "MysqlMgr.h"
#include "const.h"
#include "RedisMgr.h"

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
	//��0��Ϊ1����֪ͨ�ź�
	if (_msg_que.size() == 1) {
		unique_lk.unlock();
		_consume.notify_one();
	}
}

void LogicSystem::DealMsg() {
	for (;;) {
		std::unique_lock<std::mutex> unique_lk(_mutex);
		//�ж϶���Ϊ�������������������ȴ������ͷ���
		while (_msg_que.empty() && !_b_stop) {
			_consume.wait(unique_lk);
		}

		//�ж��Ƿ�Ϊ�ر�״̬���������߼�ִ��������˳�ѭ��
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

		//���û��ͣ������˵��������������
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
	//auto uid = root["uid"].asInt();
	int uid = 12;
	auto token = root["token"].asString();
	std::cout << "user login uid is  " << uid << " user token  is "
		<< token << endl;
	auto rsp = StatusGrpcClient::GetInstance().Login(uid, token);
	Json::Value  rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, MSG_CHAT_LOGIN_RSP);
		});

	//��redis��ȡ�û�token�Ƿ���ȷ
	//std::string uid_str = std::to_string(uid);
	//std::string token_key = USERTOKENPREFIX + uid_str;
	//std::string token_value = "";
	//bool success = RedisMgr::GetInstance().Get(token_key, token_value);
	//if (!success) {
	//	rtvalue["error"] = ErrorCodes::UidInvalid;
	//	return ;
	//}

	//if (token_value != token) {
	//	rtvalue["error"] = ErrorCodes::TokenInvalid;
	//	return ;
	//}

	rtvalue["error"] = rsp.error();
	if (rsp.error() != ErrorCodes::Success) {
		return;
	}

	//std::string base_key = USER_BASE_INFO + uid_str;
	//auto user_info = std::make_shared<UserInfo>();
	//bool b_base = GetBaseInfo(base_key, uid, user_info);
	//if (!b_base) {
	//	rtvalue["error"] = ErrorCodes::UidInvalid;
	//	return;
	//}
	std::shared_ptr<UserInfo> user_info = nullptr;	
	user_info = MysqlMgr::GetInstance().GetUser(uid);
	rtvalue["uid"] = uid;
	//rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	//rtvalue["nick"] = user_info->nick;
	//rtvalue["desc"] = user_info->desc;
	//rtvalue["sex"] = user_info->sex;
	//rtvalue["icon"] = user_info->icon;

	auto server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
	//����¼��������
	auto rd_res = RedisMgr::GetInstance().HGet(LOGIN_COUNT, server_name);
	int count = 0;
	if (!rd_res.empty()) {
		count = std::stoi(rd_res);
	}

	count++;
	auto count_str = std::to_string(count);
	RedisMgr::GetInstance().HSet(LOGIN_COUNT, server_name, count_str);
	//session���û�uid
	session->SetUserId(uid);
	//Ϊ�û����õ�¼ip server������
	//std::string  ipkey = USERIPPREFIX + uid_str;
	//RedisMgr::GetInstance().Set(ipkey, server_name);
	//uid��session�󶨹���,�����Ժ����˲���
	//UserMgr::GetInstance().SetUserSession(uid, session);

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
