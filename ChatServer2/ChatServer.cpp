#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOServicePool.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "ChatServiceImpl.h"
using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;
int main()
{
    try {
        auto& cfg = ConfigMgr::Inst();
        std::string server_name = cfg["SelfServer"]["Name"];
        auto& pool = AsioIOServicePool::GetInstance();
        RedisMgr::GetInstance().HSet(LOGIN_COUNT, server_name, "0");
        // grpc server
        std::string grpc_server_address(cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"]);
        ChatServiceImpl service;
        grpc::ServerBuilder builder;
        builder.AddListeningPort(grpc_server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "RPC Server listening on " << grpc_server_address << std::endl;
        std::thread grpc_server_thread([&server]() {
            server->Wait();
            });

        boost::asio::io_context  io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context, &pool](auto, auto) {
            io_context.stop();
            pool.Stop(); 
            });
        auto port_str = cfg["SelfServer"]["Port"];
        CServer s(io_context, atoi(port_str.c_str()));
        io_context.run();
        RedisMgr::GetInstance().HDel(LOGIN_COUNT, server_name);
        RedisMgr::GetInstance().Close();
        grpc_server_thread.join();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << endl;
    }
}