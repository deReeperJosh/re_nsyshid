
#include "websock_chat.h"

#include <iomanip>

std::mutex gUserControlMutex, gSessionControlMutex;

std::set<std::string> gUserNames;
std::vector<User> gUsers;
std::map<std::string, size_t> gUserNameIndex;
std::map<std::string, size_t> gUserSessions;

User& addUser(std::string username, std::string password, std::string displayName) {
    gUserControlMutex.lock();
    gUserNames.insert(username);

    User u = {
        .id             = gUsers.size(),
        .username       = std::move(username),
        .displayName    = std::move(displayName)
    };
    
    u.setPassword(password);
    gUserNameIndex.insert({u.username, u.id});
    gUsers.push_back(std::move(u));
    
    User& res = gUsers.back();
    gUserControlMutex.unlock();
    return res;
}

std::string createSession(size_t userId) {
    std::stringstream ss;

    auto& hexStream = ss << std::hex << std::setfill('0');

    for (size_t i = 0; i < 20; ++i) 
        hexStream << std::setw(2) << static_cast<unsigned>(rand() % 256);

    std::string token = ss.str();

    gSessionControlMutex.lock();
    gUserSessions.insert({token, userId});
    gSessionControlMutex.unlock();

    std::cout << "New session:     <" << token << '>' << std::endl;

    return token;
}

void destroySession(const std::string& token) {
    std::cout << "Destroy session <" << token << '>' << std::endl;
    gSessionControlMutex.lock();
    gUserSessions.erase(token);
    gSessionControlMutex.unlock();
}

std::string parseSessionCookie(std::string cookieString) {
    ssize_t pos = cookieString.find("tinyhttpChatSess=");

    if (pos < 0)
        return {};
    
    std::string croppedCookie = cookieString.substr(pos);

    if (croppedCookie.length() <= strlen("tinyhttpChatSess="))
        return {};

    croppedCookie = croppedCookie.substr(croppedCookie.find('=') + 1);
    croppedCookie = croppedCookie.substr(0, croppedCookie.find(';'));
    return croppedCookie;
}
