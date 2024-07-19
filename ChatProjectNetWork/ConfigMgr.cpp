#include "ConfigMgr.h"
#include "const.h"
void TestRedis() {
    //连接redis 需要启动才可以进行连接
//redis默认监听端口为6387 可以再配置文件中修改
    redisContext* c = redisConnect("127.0.0.1", 6380);
    if (c->err)
    {
        printf("Connect to redisServer faile:%s\n", c->errstr);
        redisFree(c);        return;
    }
    printf("Connect to redisServer Success\n");
    std::string redis_password = "123456";
    redisReply* r = (redisReply*)redisCommand(c, "AUTH %s", redis_password.c_str());
    if (r->type == REDIS_REPLY_ERROR) {
        printf("Redis认证失败！\n");
    }
    else {
        printf("Redis认证成功！\n");
    }
    //为redis设置key
    const char* command1 = "set stest1 value1";
    //执行redis命令行
    r = (redisReply*)redisCommand(c, command1);
    //如果返回NULL则说明执行失败
    if (NULL == r)
    {
        printf("Execut command1 failure\n");
        redisFree(c);        return;
    }
    //如果执行失败则释放连接
    if (!(r->type == REDIS_REPLY_STATUS && (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
    {
        printf("Failed to execute command[%s]\n", command1);
        freeReplyObject(r);
        redisFree(c);        return;
    }
    //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command1);
    const char* command2 = "strlen stest1";
    r = (redisReply*)redisCommand(c, command2);
    //如果返回类型不是整形 则释放连接
    if (r->type != REDIS_REPLY_INTEGER)
    {
        printf("Failed to execute command[%s]\n", command2);
        freeReplyObject(r);
        redisFree(c);        return;
    }
    //获取字符串长度
    int length = r->integer;
    freeReplyObject(r);
    printf("The length of 'stest1' is %d.\n", length);
    printf("Succeed to execute command[%s]\n", command2);
    //获取redis键值对信息
    const char* command3 = "get stest1";
    r = (redisReply*)redisCommand(c, command3);
    if (r->type != REDIS_REPLY_STRING)
    {
        printf("Failed to execute command[%s]\n", command3);
        freeReplyObject(r);
        redisFree(c);        return;
    }
    printf("The value of 'stest1' is %s\n", r->str);
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command3);
    const char* command4 = "get stest2";
    r = (redisReply*)redisCommand(c, command4);
    if (r->type != REDIS_REPLY_NIL)
    {
        printf("Failed to execute command[%s]\n", command4);
        freeReplyObject(r);
        redisFree(c);        return;
    }
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command4);
    //释放连接资源
    redisFree(c);
}
ConfigMgr::ConfigMgr(const ConfigMgr& config) {
    m_config_map = config.m_config_map;
} 
ConfigMgr::~ConfigMgr() {
    m_config_map.clear();
}

ConfigMgr::ConfigMgr(){
    // 获取当前工作目录  
    boost::filesystem::path current_path = boost::filesystem::current_path();
    // 构建config.ini文件的完整路径  
    boost::filesystem::path config_path = current_path / "config.ini";
    log4cpp::Category::getInstance("server").info("配置文件路径:"+ config_path.string());
    // 使用Boost.PropertyTree来读取INI文件  
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(config_path.string(), pt);
    log4cpp::Category::getInstance("server").info("正在解析配置文件......");
    // 遍历INI文件中的所有section  
    for (const auto& section_pair : pt) {
        const std::string& section_name = section_pair.first;
        const boost::property_tree::ptree& section_tree = section_pair.second;
        // 对于每个section，遍历其所有的key-value对  
        std::map<std::string, std::string> section_config;
        for (const auto& key_value_pair : section_tree) {
            const std::string& key = key_value_pair.first;
            const std::string& value = key_value_pair.second.get_value<std::string>();
            section_config[key] = value;
        }
        SectionInfo sectionInfo;
        sectionInfo.m_section_data = section_config;
        // 将section的key-value对保存到config_map中  
        m_config_map[section_name] = sectionInfo;
    }
    // 输出所有的section和key-value对  
    for (const auto& section_entry : m_config_map) {
        const std::string& section_name = section_entry.first;
        SectionInfo section_config = section_entry.second;
        log4cpp::Category::getInstance("server").info("["+section_name +"]");
        //std::cout <<  << std::endl;
        for (const auto& key_value_pair : section_config.m_section_data) {
            log4cpp::Category::getInstance("server").info(key_value_pair.first + "=" + key_value_pair.second);
            //std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
        }
    }

}
