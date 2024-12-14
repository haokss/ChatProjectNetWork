#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include "../data.h"
#include "../Util/Utility.hpp"
#include "../RedisManager/RedisMgr.h"
#include "../MySqlConnect/MySqlMgr.h"
ChatServiceImpl::ChatServiceImpl()
{
}

Status ChatServiceImpl::NotifyAddFriend(grpc::ServerContext* context, const::message::AddFriendReq* request, ::message::AddFriendRsp* response){
    auto touid = request->touid();
    auto session = UserMgr::GetInstance().getSession(touid);
    Defer defer([request, response]() {
        response->set_error(ErrorCodes::Success);
        response->set_applyuid(request->applyuid());
        response->set_touid(request->touid());
        });
    if (session == nullptr) {
        return Status::OK;
    }
}

Status ChatServiceImpl::NotifyAuthFriend(::grpc::ServerContext* context, const::message::AuthFriendReq* request, ::message::AuthFriendRsp* response)
{
    auto touid = request->touid();
    auto session = UserMgr::GetInstance().getSession(touid);
    Defer defer([request, response]() {
        response->set_error(ErrorCodes::Success);
        response->set_fromuid(request->fromuid());
        response->set_touid(request->touid());
        });
    if (session == nullptr) {
        return Status::OK;
    }

    return Status();
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext* context, const::message::TextChatMsgReq* request, ::message::TextChatMsgRsp* response)
{
	return Status();
}

bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& user_info)
{
	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = RedisMgr::GetInstance().Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		user_info->uid = root["uid"].asInt();
		user_info->name = root["name"].asString();
		user_info->pwd = root["pwd"].asString();
		user_info->email = root["email"].asString();
		user_info->nick = root["nick"].asString();
		user_info->desc = root["desc"].asString();
		user_info->sex = root["sex"].asInt();
		user_info->icon = root["icon"].asString();
		std::cout << "user login uid is  " << user_info->uid << " name  is "
			<< user_info->name << " pwd is " << user_info->pwd << " email is " << user_info->email << endl;
	}
	else {
		//redis中没有则查询mysql
		//查询数据库
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = MysqlMgr::GetInstance().GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}

		user_info = user_info;

		//将数据库内容写入redis缓存
		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pwd"] =   user_info->pwd;
		redis_root["name"] =  user_info->name;
		redis_root["email"] = user_info->email;
		redis_root["nick"] =  user_info->nick;
		redis_root["desc"] =  user_info->desc;
		redis_root["sex"] =   user_info->sex;
		redis_root["icon"] =  user_info->icon;
		RedisMgr::GetInstance().Set(base_key, redis_root.toStyledString());
	}
}
