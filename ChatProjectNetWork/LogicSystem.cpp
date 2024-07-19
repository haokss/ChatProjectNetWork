#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"
#include "MySqlMgr.h"
#include "Utility.h"
LogicSystem::LogicSystem(){
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		beast::ostream(connection->m_response.body())<<"receive get_test req\n";
		int i = 0;
		for (auto& elem : connection->m_get_params) {
			i++;
			beast::ostream(connection->m_response.body()) << "param " << i << " key is "
				<< elem.first<<std::endl;
			beast::ostream(connection->m_response.body()) << "param " << i << " value is "
				<< elem.second << std::endl;
		}
		});

	RegPost("/get_verifycode", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->m_request.body().data());
		log4cpp::Category::getInstance("server").info("receive body is : "+body_str);
		connection->m_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			log4cpp::Category::getInstance("server").error("Failed to parse Json data!");
			root["error"] = ErrorCodes::ERROR_JSON;
			std::string json_str = root.toStyledString();
			beast::ostream(connection->m_response.body()) << json_str;
			return true;
		}
		if (!src_root.isMember("email")) {
			log4cpp::Category::getInstance("server").error("Failed to parse JSON data!");
			root["error"] = ErrorCodes::ERROR_JSON;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		GetVarifyRsp rsp = VerifyGrpcClient::GetInstance().GetVrifyCode(email);
		log4cpp::Category::getInstance("server").info("email is "+ email);
		root["error"] = rsp.error();
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->m_response.body()) << jsonstr;
		return true;

		});
	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->m_request.body().data());
		log4cpp::Category::getInstance("server").info("receive body is " + jsonSimpleStr(body_str));
		connection->m_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			log4cpp::Category::getInstance("server").error("Failed to parse JSON data!");
			root["error"] = ErrorCodes::ERROR_JSON;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		std::string name = src_root["user"].asString();
		std::string email = src_root["email"].asString();
		std::string pwd = src_root["passwd"].asString();
		std::string confirm = src_root["confirm"].asString();
		// TODO �ж����������Ƿ���ͬ
		if (pwd != confirm) {
			log4cpp::Category::getInstance("server").error("password is node same");
			root["error"] = ErrorCodes::PASSWDERR;
			std::string json_str = root.toStyledString();
			beast::ostream(connection->m_response.body()) << json_str;
		}
		//�Ȳ���redis��email��Ӧ����֤���Ƿ����
		std::string  verify_code;
		//log4cpp::Category::getInstance("server").info(CODE_PREFIX + src_root["email"].asString());
		bool b_get_verify = RedisMgr::GetInstance().Get(CODE_PREFIX + src_root["email"].asString(), verify_code);
		if (!b_get_verify) {
			log4cpp::Category::getInstance("server").error("get verify code expired");
			root["error"] = ErrorCodes::verify_EXPRIRED;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		if (verify_code != src_root["verifycode"].asString()) {
			log4cpp::Category::getInstance("server").error("verify code error");
			root["error"] = ErrorCodes::verify_CODE_ERROR;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		//����redis����
		//bool b_usr_exist = RedisMgr::GetInstance().ExistsKey(src_root["user"].asString());
		//if (b_usr_exist) {
		//	std::cout << " user exist" << std::endl;
		//	root["error"] = ErrorCodes::USRE_EXIST;
		//	std::string jsonstr = root.toStyledString();
		//	beast::ostream(connection->m_response.body()) << jsonstr;
		//	return true;
		//}
		//�������ݿ��ж��û��Ƿ����
		int uid = MySqlMgr::GetInstance().RegUser(name, email, pwd);
		if (uid == 0 || uid == -1) {
			log4cpp::Category::getInstance("server").info("user is exist");
			root["error"] = ErrorCodes::USRE_EXIST;
			beast::ostream(connection->m_response.body()) << root.toStyledString();
		}
		root["error"] = 0;
		root["email"] = email;
		root["uid"] = uid;
		root["user"] = name;
		root["passwd"] = pwd;
		root["confirm"] = confirm;
		root["verifycode"] = src_root["verifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->m_response.body()) << jsonstr;
		return true;
		});

}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con){
	if (m_get_handlers.find(path) == m_get_handlers.end()) {
		return false;
	}
	m_get_handlers[path](con);
	return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
	if (m_post_handlers.find(path) == m_post_handlers.end()) {
		return false;
	}
	m_post_handlers[path](con);

	return true;
}

void LogicSystem::RegGet(std::string url, HttpHandler handler){
	m_get_handlers.insert(std::make_pair(url, handler));

}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
	m_post_handlers.insert(std::make_pair(url, handler));
}

LogicSystem::~LogicSystem()
{
}

