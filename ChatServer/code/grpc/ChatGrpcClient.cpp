#include "ChatGrpcClient.h"

ChatGrpcClient::ChatGrpcClient()
{
    auto& cfg = ConfigMgr::Inst();
    auto server_list = cfg["PeerServer"]["Servers"];
    std::vector<std::string> words;
    std::stringstream ss(server_list);
    std::string word;
    // 可以使用，分割server
    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    } 
    for (auto& word : words) {
        if (cfg[word]["Name"].empty()) {
            continue;
        }
        _pools[cfg[word]["Name"]] = std::make_unique<ChatConPool>
            (5, cfg[word]["Host"], cfg[word]["Port"]);

    }
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req)
{
    return AddFriendRsp();
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req)
{
    return AuthFriendRsp();
}

bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
    return false;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue)
{
    return TextChatMsgRsp();
}

