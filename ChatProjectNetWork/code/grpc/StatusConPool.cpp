#include "StatusConPool.h"

StatusConPool::StatusConPool(size_t poolSize, std::string host, std::string port)
    : poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
    for (size_t i = 0; i < poolSize_; ++i) {
        std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
            grpc::InsecureChannelCredentials());
        connections_.push(StatusService::NewStub(channel));
    }
}

StatusConPool::~StatusConPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    Close();  
    while (!connections_.empty()) {
        connections_.pop();
    }
}


void StatusConPool::returnConnection(std::unique_ptr<StatusService::Stub> context) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (b_stop_) {
        return;
    }
    connections_.push(std::move(context));
    cond_.notify_one();
}

void StatusConPool::Close() {
    b_stop_ = true;
    cond_.notify_all();
}
