#include "AsioIOServicePool.h"


AsioIOServicePool::AsioIOServicePool(std::size_t size)
	:m_vec_ioc(size),
	m_vec_workPtr(size), 
	m_nextIOService(0) 
{
	// ��work�󶨵�io_context
	for (std::size_t i = 0; i < size; ++i) 
	{
		m_vec_workPtr[i] = std::unique_ptr<Work>(new Work(m_vec_ioc[i]));
	}
	// ����io_service,��������̣߳�����ÿ���̶߳�����ioc
	for (std::size_t i = 0; i < m_vec_ioc.size(); ++i) 
	{
		m_threads.emplace_back([this, i] 
		{
			m_vec_ioc[i].run();
		});
	}
}


AsioIOServicePool::~AsioIOServicePool()
{
	Stop();
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	auto& service = m_vec_ioc[m_nextIOService++];
	if (m_nextIOService == m_vec_ioc.size()) {
		m_nextIOService = 0;
	}
	return service;
}

void AsioIOServicePool::Stop()
{
	// ����ÿһ��ָ��work��unique_ptrָ��
	for (auto& work : m_vec_workPtr) {
		work.reset();
	}
	// �ȴ��߳̽���
	for (auto& t : m_threads) {
		t.join();
	}
}
