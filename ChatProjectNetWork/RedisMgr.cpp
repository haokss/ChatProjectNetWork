#include "RedisMgr.h"
#include "ConfigMgr.h"
RedisMgr::RedisMgr(){
    auto& gCfgMgr = ConfigMgr::Inst();
    std::string host = gCfgMgr["Redis"]["Host"];
    std::string port = gCfgMgr["Redis"]["Port"];
    std::string pwd = gCfgMgr["Redis"]["Passwd"];
    p_con_pool = std::make_unique<RedisConPool>(5, host.c_str(), std::stoi(port), pwd.c_str());
}

RedisMgr::~RedisMgr(){
    Close();
}

//bool RedisMgr::Connect(const std::string& host, int port){
//    auto connect = p_con_pool->getConnection();
//    if (connect == nullptr) {
//        return false;
//    }
//    //connect = redisConnect(host.c_str(), port);
//    if (connect != NULL && connect->err)
//    {
//        std::cout << "connect error " << connect->errstr << std::endl;
//        return false;
//    }
//    return true;
//}

bool RedisMgr::Get(const std::string& key, std::string& value){

    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    this->_reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());
    if (this->_reply == NULL) {
        log4cpp::Category::getInstance("server").infoStream() << "[ GET  " << key << " ] failed, ��ȡreids��֤�����";
        freeReplyObject(this->_reply);
        return false;
    }
    if (this->_reply->type != REDIS_REPLY_STRING) {
        log4cpp::Category::getInstance("server").infoStream() << "reply type:" << this->_reply->type 
        << "[ GET  " << key << " ] failed, �û��ύ��֤���ѹ���";
        freeReplyObject(this->_reply);
        return false;
    }
    value = this->_reply->str;
    freeReplyObject(this->_reply);
    log4cpp::Category::getInstance("server").infoStream()<<"Succeed to execute command [ GET " << key << "  ]";
    return true;
}

bool RedisMgr::Set(const std::string& key, const std::string& value){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    //ִ��redis������
    this->_reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());
    //�������NULL��˵��ִ��ʧ��
    if (NULL == this->_reply)
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    //���ִ��ʧ�����ͷ�����
    if (!(this->_reply->type == REDIS_REPLY_STATUS && (strcmp(this->_reply->str, "OK") == 0 || strcmp(this->_reply->str, "ok") == 0)))
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
    freeReplyObject(this->_reply);
    std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
    return true;
}

bool RedisMgr::Auth(const std::string& password){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    this->_reply = (redisReply*)redisCommand(connect, "AUTH %s", password.c_str());
    if (this->_reply->type == REDIS_REPLY_ERROR) {
        log4cpp::Category::getInstance("server").error("���ݿ������֤ʧ�ܣ�");
        //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
        freeReplyObject(this->_reply);
        return false;
    }
    else {
        //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
        freeReplyObject(this->_reply);
        log4cpp::Category::getInstance("server").info("��֤�ɹ���");
        return true;
    }
}

bool RedisMgr::LPush(const std::string& key, const std::string& value){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    this->_reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == this->_reply)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    this->_reply = (redisReply*)redisCommand(connect, "LPOP %s ", key.c_str());
    if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ LPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    value = _reply->str;
    std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    this->_reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == this->_reply)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    this->_reply = (redisReply*)redisCommand(connect, "RPOP %s ", key.c_str());
    if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    value = _reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    this->_reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    const char* argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;
    this->_reply = (redisReply*)redisCommandArgv(connect, 4, argv, argvlen);
    if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& hkey){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return std::string("");
    }
    const char* argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    this->_reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);
    if (this->_reply == nullptr || this->_reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(this->_reply);
        std::cout << "Execut command [ HGet " << key << " " << hkey << "  ] failure ! " << std::endl;
        return "";
    }
    std::string value = this->_reply->str;
    freeReplyObject(this->_reply);
    std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
    return value;
}

bool RedisMgr::Del(const std::string& key){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    this->_reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());
    if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

bool RedisMgr::ExistsKey(const std::string& key){
    auto connect = p_con_pool->getConnection();
    if (connect == nullptr) {
        return false;
    }
    this->_reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());
    if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer == 0) {
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

void RedisMgr::Close(){
    //redisFree(_connect);
    p_con_pool->close();
}

