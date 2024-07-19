#pragma once
#include "const.h"
#include "Singleton.hpp"
#include "RedisConPool.h"
class RedisMgr:public Singleton<RedisMgr>, public std::enable_shared_from_this<RedisMgr>
{
	friend class Singleton<RedisMgr>;
public:
    ~RedisMgr();
    //bool Connect(const std::string& host, int port);
    bool Get(const std::string& key, std::string& value);
    bool Set(const std::string& key, const std::string& value);
    bool Auth(const std::string& password);
    bool LPush(const std::string& key, const std::string& value);
    bool LPop(const std::string& key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string HGet(const std::string& key, const std::string& hkey);
    bool Del(const std::string& key);
    bool ExistsKey(const std::string& key);
    void Close();
private:
    RedisMgr();
    //redisContext* _connect;
    std::unique_ptr<RedisConPool> p_con_pool;
    redisReply* _reply;
};

//void TestRedisMgr() {
//	//assert(RedisMgr::GetInstance().Connect("127.0.0.1", 6380));
//	//assert(RedisMgr::GetInstance().Auth("123456"));
//	assert(RedisMgr::GetInstance().Set("blogwebsite", "llfc.club"));
//	std::string value = "";		  
//	assert(RedisMgr::GetInstance().Get("blogwebsite", value));
//	assert(RedisMgr::GetInstance().Get("nonekey", value) == false);
//	assert(RedisMgr::GetInstance().HSet("bloginfo", "blogwebsite", "llfc.club"));
//	assert(RedisMgr::GetInstance().HGet("bloginfo", "blogwebsite") != "");
//	assert(RedisMgr::GetInstance().ExistsKey("bloginfo"));
//	assert(RedisMgr::GetInstance().Del("bloginfo"));
//	assert(RedisMgr::GetInstance().Del("bloginfo"));
//	assert(RedisMgr::GetInstance().ExistsKey("bloginfo") == false);
//	assert(RedisMgr::GetInstance().LPush("lpushkey1", "lpushvalue1"));
//	assert(RedisMgr::GetInstance().LPush("lpushkey1", "lpushvalue2"));
//	assert(RedisMgr::GetInstance().LPush("lpushkey1", "lpushvalue3"));
//	assert(RedisMgr::GetInstance().RPop("lpushkey1", value));
//	assert(RedisMgr::GetInstance().RPop("lpushkey1", value));
//	assert(RedisMgr::GetInstance().LPop("lpushkey1", value));
//	assert(RedisMgr::GetInstance().LPop("lpushkey2", value) == false);
//	RedisMgr::GetInstance().Close();
//}