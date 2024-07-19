#pragma once
#include "const.h"
#include "MysqlDao.h"
#include "Singleton.hpp"
class MySqlMgr : public Singleton<MySqlMgr>
{
	friend class Singleton<MySqlMgr>;
public:
	~MySqlMgr();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
private:
	MySqlMgr();

private:
	MysqlDao m_dao;
};
