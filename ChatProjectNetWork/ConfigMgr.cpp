#include "ConfigMgr.h"
#include "const.h"
void TestRedis() {
    //����redis ��Ҫ�����ſ��Խ�������
//redisĬ�ϼ����˿�Ϊ6387 �����������ļ����޸�
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
        printf("Redis��֤ʧ�ܣ�\n");
    }
    else {
        printf("Redis��֤�ɹ���\n");
    }
    //Ϊredis����key
    const char* command1 = "set stest1 value1";
    //ִ��redis������
    r = (redisReply*)redisCommand(c, command1);
    //�������NULL��˵��ִ��ʧ��
    if (NULL == r)
    {
        printf("Execut command1 failure\n");
        redisFree(c);        return;
    }
    //���ִ��ʧ�����ͷ�����
    if (!(r->type == REDIS_REPLY_STATUS && (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
    {
        printf("Failed to execute command[%s]\n", command1);
        freeReplyObject(r);
        redisFree(c);        return;
    }
    //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command1);
    const char* command2 = "strlen stest1";
    r = (redisReply*)redisCommand(c, command2);
    //����������Ͳ������� ���ͷ�����
    if (r->type != REDIS_REPLY_INTEGER)
    {
        printf("Failed to execute command[%s]\n", command2);
        freeReplyObject(r);
        redisFree(c);        return;
    }
    //��ȡ�ַ�������
    int length = r->integer;
    freeReplyObject(r);
    printf("The length of 'stest1' is %d.\n", length);
    printf("Succeed to execute command[%s]\n", command2);
    //��ȡredis��ֵ����Ϣ
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
    //�ͷ�������Դ
    redisFree(c);
}
ConfigMgr::ConfigMgr(const ConfigMgr& config) {
    m_config_map = config.m_config_map;
} 
ConfigMgr::~ConfigMgr() {
    m_config_map.clear();
}

ConfigMgr::ConfigMgr(){
    // ��ȡ��ǰ����Ŀ¼  
    boost::filesystem::path current_path = boost::filesystem::current_path();
    // ����config.ini�ļ�������·��  
    boost::filesystem::path config_path = current_path / "config.ini";
    log4cpp::Category::getInstance("server").info("�����ļ�·��:"+ config_path.string());
    // ʹ��Boost.PropertyTree����ȡINI�ļ�  
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(config_path.string(), pt);
    log4cpp::Category::getInstance("server").info("���ڽ��������ļ�......");
    // ����INI�ļ��е�����section  
    for (const auto& section_pair : pt) {
        const std::string& section_name = section_pair.first;
        const boost::property_tree::ptree& section_tree = section_pair.second;
        // ����ÿ��section�����������е�key-value��  
        std::map<std::string, std::string> section_config;
        for (const auto& key_value_pair : section_tree) {
            const std::string& key = key_value_pair.first;
            const std::string& value = key_value_pair.second.get_value<std::string>();
            section_config[key] = value;
        }
        SectionInfo sectionInfo;
        sectionInfo.m_section_data = section_config;
        // ��section��key-value�Ա��浽config_map��  
        m_config_map[section_name] = sectionInfo;
    }
    // ������е�section��key-value��  
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
