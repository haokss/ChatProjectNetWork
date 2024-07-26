#pragma once
#include "const.h"
#include "ConfigMgr.h"
#include <grpcpp/grpcpp.h>
#include "Singleton.hpp"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "StatusConPool.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;
using message::LoginReq;
using message::LoginRsp;

class StatusGrpcClient :public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;
public:
    ~StatusGrpcClient() {}
    GetChatServerRsp GetChatServer(int uid);
    LoginRsp Login(int uid, std::string token);
private:
    StatusGrpcClient();
    std::unique_ptr<StatusConPool> pool_;
};