#include "UserMgr.h"
UserMgr::UserMgr(){
    
}
UserMgr::~UserMgr(){
    m_uids.clear();
}

std::shared_ptr<CSession> UserMgr::getSession(int uid)
{
    std::lock_guard <std::mutex> lock(m_session_mtx);
    auto session = m_uids.find(uid);
    if (session == m_uids.end()) {
        return nullptr;
    }
    return session->second;
}

void UserMgr::setUserSession(int uid, std::shared_ptr<CSession> session){
    std::lock_guard<std::mutex> lock(m_session_mtx);
    m_uids[uid] = session;
}

void UserMgr::removeUserSession(int uid){
    std::lock_guard<std::mutex> lock(m_session_mtx);
    m_uids.erase(uid);
}


