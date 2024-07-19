#include "MySqlPool.h"
#include "Utility.h"
#include "const.h"
MySqlPool::MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize)
    : url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolSize), b_stop_(false) {
    try {
        for (int i = 0; i < poolSize_; ++i) {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            //std::unique_ptr<sql::Connection> con(driver->connect(url_, user_, pass_));
            //std::cerr << url_ << ":" << user_ << ":" << pass_ << std::endl;
            auto *con(driver->connect(url, user_, pass_));
            con->setSchema(schema_);
            auto current_time = std::chrono::system_clock::now().time_since_epoch();
            long long time_stamp = std::chrono::duration_cast<std::chrono::seconds>(current_time).count();
            pool_.push(std::make_unique<SqlConnection>(con, time_stamp));
        }
        _check_thread = std::thread([this]() {
            while (!b_stop_) {
                checkConnection();
                std::this_thread::sleep_for(std::chrono::seconds(60));
            }
            });
        _check_thread.detach();
    }
    catch (sql::SQLException& e) {
        // 处理异常
        log4cpp::Category::getInstance("server").errorStream()<< "mysql connect error:"<<e.what();
    }
}

std::unique_ptr<SqlConnection> MySqlPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { 
        return b_stop_ || !pool_.empty();
        });
    if (b_stop_) {
        return nullptr;
    }
    std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
    pool_.pop();
    return con;
}

void MySqlPool::returnConnection(std::unique_ptr<SqlConnection> con) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (b_stop_) {
        return;
    }
    pool_.push(std::move(con));
    cond_.notify_one();
}

void MySqlPool::Close() {
    b_stop_ = true;
    cond_.notify_all();
}

MySqlPool::~MySqlPool() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!pool_.empty()) {
        pool_.pop();
    }
}

void MySqlPool::checkConnection(){
    std::lock_guard<std::mutex> lock(mutex_);
    int pool_size = pool_.size();
    auto current_time = std::chrono::system_clock::now().time_since_epoch();
    long long time_stamp = std::chrono::duration_cast<std::chrono::seconds>(current_time).count();
    for (int i = 0; i < pool_size; i++) {
        auto con = std::move(pool_.front());
        pool_.pop();
        Defer defer([this, &con]() {
            pool_.push(std::move(con));
            });
        
        if (time_stamp - con->_last_oper_time < 500) {
            continue;
        }
        try {
            std::unique_ptr<sql::Statement> statement(con->_con->createStatement());
            statement->executeQuery("SELECT 1");
            con->_last_oper_time = time_stamp;
        }
        catch (sql::SQLException& e) {
            log4cpp::Category::getInstance("server").error(e.what());
            // 重新创建链接
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            auto* new_con = driver->connect(url_, user_, pass_);
            new_con->setSchema(schema_);
            con->_con.reset(new_con);
            con->_last_oper_time = time_stamp;
        }
    }
}
