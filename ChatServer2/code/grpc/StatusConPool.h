#pragma once
#include "../const.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "../Util/Singleton.hpp"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::StatusService;
#include <atomic>
class StatusConPool
{
public:
    StatusConPool(size_t poolSize, std::string host, std::string port);
    ~StatusConPool();
    std::unique_ptr<StatusService::Stub> getConnection();
    void returnConnection(std::unique_ptr<StatusService::Stub> context);
    void Close();
private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<StatusService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

inline std::unique_ptr<StatusService::Stub> StatusConPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] {
        return b_stop_ || !connections_.empty();
        });
    //如果停止则直接返回空指针
    if (b_stop_) {
        return  nullptr;
    }
    auto context = std::move(connections_.front());
    connections_.pop();
    return context;
}
