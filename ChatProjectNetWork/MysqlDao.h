#pragma once
#include "const.h"
#include "MySqlPool.h"

// Êý¾Ý¿â¶ÁÐ´
class MysqlDao
{
public:
    MysqlDao();
    ~MysqlDao();
    int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
private:
    std::unique_ptr<MySqlPool> pool_;
};

