#include "MySqlMgr.h"

MySqlMgr::MySqlMgr()
{
}

MySqlMgr::~MySqlMgr()
{
}

int MySqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    
    return m_dao.RegUser(name, email, pwd);
}

bool MySqlMgr::checkEmail(const std::string& name, const std::string& email){
    return m_dao.checkEmail(name, email);
}

bool MySqlMgr::updatePasswd(const std::string& name, const std::string& pwd){
    return m_dao.updatePasswd(name, pwd);
}

bool MySqlMgr::checkPwd(const std::string& name, const std::string& pwd, _RPC_ASYNC_STATE::UserInfo& user_info)
{
    return m_dao.checkPwd(name, pwd, user_info);
}
