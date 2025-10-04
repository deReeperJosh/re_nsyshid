
#ifndef WEBSOCK_CHAT_H
#define WEBSOCK_CHAT_H

#include <http.hpp>

#include <set>
#include <mutex>

// use SHA1 from the websocket implementation
extern void hash_sha1(const void* dataptr, const size_t size, uint8_t* outBuffer);

struct User {
    size_t id;
    std::string username;
    std::string displayName;
    uint8_t password[20];

    void setPassword(const std::string& pass) {
        hash_sha1(pass.data(), pass.size(), password);
    }

    bool checkPassword(const std::string& pass) {
        uint8_t hash[20];
        hash_sha1(pass.data(), pass.size(), hash);
        return memcmp(hash, password, 20) == 0;
    }
};

struct ChatSocketHandler : public WebsockClientHandler {
    void onConnect() override;
    void onTextMessage(const std::string& message) override;
    void onBinaryMessage(const std::vector<uint8_t>& data) override;
    void onDisconnect() override;

    private:
        std::string mSessionToken;
        User* mUser;

        bool checkSession();
};

extern std::set<std::string>         gUserNames;
extern std::vector<User>             gUsers;
extern std::map<std::string, size_t> gUserNameIndex;
extern std::map<std::string, size_t> gUserSessions;

User& addUser(std::string username, std::string password, std::string displayName);
std::string createSession(size_t userId);
void        destroySession(const std::string& token);
std::string parseSessionCookie(std::string cookieString);

#endif
