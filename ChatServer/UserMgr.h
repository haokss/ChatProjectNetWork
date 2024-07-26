#pragma once
#include "Singleton.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>
#include "CSession.h"
class UserMgr : public Singleton<UserMgr>
{
	friend class Singleton<UserMgr>;
public:
	~UserMgr();
	std::shared_ptr<CSession> getSession(int uid);
	void setUserSession(int uid, std::shared_ptr<CSession> session);
	void removeUserSession(int uid);
private:
	UserMgr();
	std::mutex m_session_mtx;
	std::unordered_map<int, std::shared_ptr<CSession>> m_uids;
};

