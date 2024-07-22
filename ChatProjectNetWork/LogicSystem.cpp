#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"
#include "MySqlMgr.h"
#include "Utility.h"
#include "StatusGrpcClient.h"
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
		// TODO 判断两个密码是否相同
		if (pwd != confirm) {
			log4cpp::Category::getInstance("server").error("password is node same");
			root["error"] = ErrorCodes::ERROR_PASSWD;
			std::string json_str = root.toStyledString();
			beast::ostream(connection->m_response.body()) << json_str;
		}
		//先查找redis中email对应的验证码是否合理
		std::string  verify_code;
		//log4cpp::Category::getInstance("server").info(CODE_PREFIX + src_root["email"].asString());
		bool b_get_verify = RedisMgr::GetInstance().Get(CODE_PREFIX + src_root["email"].asString(), verify_code);
		if (!b_get_verify) {
			log4cpp::Category::getInstance("server").error("get verify code expired");
			root["error"] = ErrorCodes::ERROR_VERIFY_EXPRIRED;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		if (verify_code != src_root["verifycode"].asString()) {
			log4cpp::Category::getInstance("server").error("verify code error");
			root["error"] = ErrorCodes::ERROR_VERIFY_CODE;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		//访问redis查找
		//bool b_usr_exist = RedisMgr::GetInstance().ExistsKey(src_root["user"].asString());
		//if (b_usr_exist) {
		//	std::cout << " user exist" << std::endl;
		//	root["error"] = ErrorCodes::USRE_EXIST;
		//	std::string jsonstr = root.toStyledString();
		//	beast::ostream(connection->m_response.body()) << jsonstr;
		//	return true;
		//}
		//查找数据库判断用户是否存在
		int uid = MySqlMgr::GetInstance().RegUser(name, email, pwd);
		if (uid == 0 || uid == -1) {
			log4cpp::Category::getInstance("server").info("user is exist");
			root["error"] = ErrorCodes::ERROR_USRE_EXIST;
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
	RegPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->m_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->m_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::ERROR_JSON;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["passwd"].asString();
		//先查找redis中email对应的验证码是否合理
		std::string  varify_code;
		bool b_get_varify = RedisMgr::GetInstance().Get(CODE_PREFIX + src_root["email"].asString(), varify_code);
		if (!b_get_varify) {
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::ERROR_VERIFY_EXPRIRED;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		if (varify_code != src_root["verifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::ERROR_VERIFY_CODE;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		//查询数据库判断用户名和邮箱是否匹配
		bool email_valid = MySqlMgr::GetInstance().checkEmail(name, email);
		if (!email_valid) {
			std::cout << " user email not match" << std::endl;
			root["error"] = ErrorCodes::ERROR_EMAIL_NOT_MATCH;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		//更新密码为最新密码
		bool b_up = MySqlMgr::GetInstance().updatePasswd(name, pwd);
		if (!b_up) {
			std::cout << " update pwd failed" << std::endl;
			root["error"] = ErrorCodes::ERROR_PASSWD_UP_FAILED;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		std::cout << "succeed to update password" << pwd << std::endl;
		root["error"] = 0;
		root["email"] = email;
		root["user"] = name;
		root["passwd"] = pwd;
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->m_response.body()) << jsonstr;
		return true;
		});
	RegPost("/user_login", [](std::shared_ptr<HttpConnection> connection) {
		std::string body_str = boost::beast::buffers_to_string(connection->m_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->m_request.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::ERROR_JSON;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_request.body()) << jsonstr;
			return true;
		}
		auto email = src_root["email"].asString();
		auto pwd = src_root["passwd"].asString();
		UserInfo userInfo;
		//查询数据库判断用户名和密码是否匹配
		bool pwd_valid = MySqlMgr::GetInstance().checkPwd(email, pwd, userInfo);
		if (!pwd_valid) {
			std::cout << " user pwd not match" << std::endl;
			root["error"] = ErrorCodes::ERROR_PASSWD;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		std::cout << "密码匹配成功" << std::endl;
		//查询StatusServer找到合适的连接
		auto reply = StatusGrpcClient::GetInstance().GetChatServer(userInfo.uid);
		if (reply.error()) {
			std::cout << " grpc get chat server failed, error is " << reply.error() << std::endl;
			root["error"] = ErrorCodes::ERROR_RPCFAILED;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->m_response.body()) << jsonstr;
			return true;
		}
		std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;
		root["error"] = 0;
		root["user"] = userInfo.name;
		root["uid"] = userInfo.uid;
		root["token"] = reply.token();
		root["host"] = reply.host();
		root["port"] = reply.port();
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

