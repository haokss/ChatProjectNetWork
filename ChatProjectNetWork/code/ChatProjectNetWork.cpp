#include <iostream>
#include "ChatServer/CServer.h"
#include <signal.h>
#include <grpcpp/grpcpp.h>
#include <assert.h>
#include "ConfigManager/ConfigMgr.h"
#include "RedisManager/RedisMgr.h"

int main()
{
	// 解析日志库属性
	std::string initFileName = "D:\\VS code\\ChatProjectNetWork\\ChatProjectNetWork\\runtime_script\\log4cpp.properties";
	log4cpp::PropertyConfigurator::configure(initFileName);

	//TestRedisMgr();
	auto &gCfgMgr = ConfigMgr::Inst();
	std::string gate_port_str = gCfgMgr["GateServer"]["Port"];
	//TestRedis();

	unsigned short gate_port = atoi(gate_port_str.c_str());
	try {
		net::io_context ioc{ 1 };
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const boost::system::error_code& ec, int signal_num)
			{
				if (ec)
				{
					return;
				}
				ioc.stop();
			});
		std::make_shared<CServer>(ioc, gate_port)->Start();
		ioc.run();
	}
	catch (std::exception& e) {
		log4cpp::Category::getInstance("main").error(e.what());
		return EXIT_FAILURE;
	}

	// 关闭日志库
	log4cpp::Category::shutdown();
	return 0;
}
