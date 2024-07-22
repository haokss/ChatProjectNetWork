#pragma once
#include "Singleton.hpp"
#include <vector>
#include <boost/asio.hpp>
#include <thread>

class AsioIOServicePool :public Singleton<AsioIOServicePool>
{
	friend class Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work; // 防止io_context没有绑定就推出
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(AsioIOServicePool&) = delete;
	// 使用轮询的方式返回io_context
	boost::asio::io_context& GetIOService();
	void Stop();
private:
	AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());
private:
	std::vector<IOService> m_vec_ioc;
	std::vector<WorkPtr> m_vec_workPtr;
	std::vector<std::thread> m_threads;
	std::size_t m_nextIOService;
};

