#pragma once
#include "const.h"
class RedisConPool
{
public:
	RedisConPool(size_t pool_size, const char* host, const int port, const char* pwd);
	~RedisConPool();
	RedisConPool(const RedisConPool&) = delete;
	RedisConPool& operator=(const RedisConPool&) = delete;
	redisContext* getConnection();
	void returnConnection(redisContext* context);
	void close();

private:
	std::queue<redisContext*> m_que_redisCons;
	std::mutex m_mtx;
	std::condition_variable m_condition_var;
	size_t m_pool_size;
	std::atomic<bool> m_stop;
	const char* m_host;
	const int m_port;
};

