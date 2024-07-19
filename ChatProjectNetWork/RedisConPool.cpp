#include "RedisConPool.h"

RedisConPool::RedisConPool(size_t pool_size, const char* host, const int port, const char* pwd)
	:m_pool_size(pool_size), m_host(host), m_port(port), m_stop(false) {
	for (size_t i = 0; i < pool_size; ++i) {
		auto* context = redisConnect(host, port);
		if (context == nullptr || context->err != 0) {
			if (context != nullptr) {
				redisFree(context);
			}
			continue;
		}
		auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
		if (reply->type == REDIS_REPLY_ERROR) {
			log4cpp::Category::getInstance("server").error("��֤ʧ��");
			freeReplyObject(reply);
			continue;
		}
		// �ͷ�redisCommandִ�к󷵻ص�redisReply�ڴ�
		freeReplyObject(reply);
		log4cpp::Category::getInstance("server").info("��֤�ɹ�");
		m_que_redisCons.push(context);
	}
}

RedisConPool::~RedisConPool(){
	std::lock_guard<std::mutex> lock(m_mtx);
	while (!m_que_redisCons.empty()) {
		m_que_redisCons.pop();
	}
}

redisContext* RedisConPool::getConnection(){
	std::unique_lock<std::mutex> lock(m_mtx);
	m_condition_var.wait(lock, [this] {
		return m_stop || !m_que_redisCons.empty();
		});
	if (m_stop) {
		return nullptr;
	}
	// ȡ��һ������
	auto* con = m_que_redisCons.front();
	m_que_redisCons.pop();
	return con;
}

void RedisConPool::returnConnection(redisContext* context){
	std::lock_guard<std::mutex> lock(m_mtx);
	if (m_stop) {
		return;
	}
	m_que_redisCons.push(context);
	m_condition_var.notify_one();
}

void RedisConPool::close(){
	m_stop = true;
	m_condition_var.notify_all();
}
