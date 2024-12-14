#pragma once
#include "../const.h"
#include <thread>

class SqlConnection {
public:
    SqlConnection(sql::Connection* con, int64_t last_time)
        :_con(con), _last_oper_time(last_time) {}
    std::unique_ptr<sql::Connection> _con;
    int64_t _last_oper_time;
};

class MySqlPool {
public:
    MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
    std::unique_ptr<SqlConnection> getConnection();
    void returnConnection(std::unique_ptr<SqlConnection> con);
    void Close();
    ~MySqlPool();
private:
    void checkConnection();
private:
    std::string url_;
    std::string user_;
    std::string pass_;
    std::string schema_; // 数据库名称
    int poolSize_;
    std::queue<std::unique_ptr<SqlConnection>> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> b_stop_;
    std::thread _check_thread; // 检测连接线程，心跳机制
};