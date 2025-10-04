
#include "websock_chat.h"

#include <signal.h>
#include <http.hpp>
#include <home.html.hpp>

static HttpResponse handleRegister(const HttpRequest& req) {
    miniJson::Json::_object res;
    auto body = req.json();
    
    if (!body.isObject()) {
        res["error"] = "INVALID_BODY";
        return {200, res};
    }

    const auto& j_username      = body["username"];
    const auto& j_password1     = body["password1"];
    const auto& j_password2     = body["password2"];
    const auto& j_displayName   = body["displayname"];

    if (!j_username.isString() || !j_password1.isString() || !j_password2.isString() || !j_displayName.isString()) {
        res["error"] = "INVALID_BODY";
        return {200, res};
    }

    std::string username    = j_username.toString();
    std::string password1   = j_password1.toString();
    std::string password2   = j_password2.toString();
    std::string displayName = j_displayName.toString();

    if (username.length() < 3) {
        res["error"] = "USERNAME_TOO_SHORT";
        return {200, res};
    }

    if (password1.length() < 8) {
        res["error"] = "PASSWORD_TOO_SHORT";
        return {200, res};
    }

    if (password2 != password2) {
        res["error"] = "PASSWORD_MISSMATCH";
        return {200, res};
    }

    if (displayName.empty()) {
        res["error"] = "DISPLAY_NAME_EMPTY";
        return {200, res};
    }

    // possible race condition here, you may be able to create two users with the same username
    if (gUserNames.find(username) != gUserNames.end()) {
        res["error"] = "ALREADY_EXISTS";
        return {200, res};
    }

    std::cout << "Register:        <" << username << '>' << std::endl;

    auto newUser = addUser(std::move(username), std::move(password1), std::move(displayName));
    
    res["error"] = nullptr;
    
    HttpResponse response{200, res};
    response["Set-Cookie"] = "tinyhttpChatSess=" + createSession(newUser.id) + "; Max-Age=2592000; path=/; HttpOnly; SameSite=Strict";
    return response;
}

static HttpResponse handleLogin(const HttpRequest& req) {
    miniJson::Json::_object res;
    auto body = req.json();
    
    if (!body.isObject()) {
        res["error"] = "INVALID_BODY";
        return {200, res};
    }

    const auto& j_username  = body["username"];
    const auto& j_password  = body["password"];

    if (!j_username.isString() || !j_password.isString()) {
        res["error"] = "INVALID_BODY";
        return {200, res};
    }

    std::string username    = j_username.toString();
    std::string password    = j_password.toString();

    auto key = gUserNameIndex.find(username);

    if (key == gUserNameIndex.end()) {
        res["error"] = "NO_USER";
        return {200, res};
    }

    size_t userId = key->second;

    if (!gUsers[userId].checkPassword(password)) {
        res["error"] = "BAD_PASSWORD";
        return {200, res};
    }

    res["error"] = nullptr;

    std::cout << "Login:           <" << username << '>' << std::endl;
    
    HttpResponse response{200, res};
    response["Set-Cookie"] = "tinyhttpChatSess=" + createSession(userId) + "; Max-Age=2592000; path=/; HttpOnly; SameSite=Strict";
    return response;
}

HttpServer s;

void handle_sigint(int sig) {
    s.shutdown();
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));

    // Add some users
    addUser("test1", "password", "Test1");
    addUser("test2", "password", "Test2");

    signal(SIGINT, handle_sigint);

    s.when("/")->serveFile("index.html");
    s.whenMatching("/static/[^/]+")->serveFromFolder("static");

    s.when("/register")->posted(handleRegister);
    s.when("/login")->posted(handleLogin);

    s.when("/dump-users")->requested([](const HttpRequest& req) -> HttpResponse {
        miniJson::Json::_object res;

        miniJson::Json::_array userNames;

        for (const auto& e : gUserNames)
            userNames.push_back(e);

        miniJson::Json::_array users;

        for (const auto& e : gUsers) {
            miniJson::Json::_object user;
            user["username"] = e.username;
            user["displayName"] = e.displayName;
            user["id"] = static_cast<int>(e.id);
            users.push_back(user);
        }

        res["userNamesInUse"] = userNames;
        res["activeUsers"] = users;

        return {200, res};
    });

    s.when("/home")->requested([](const HttpRequest& req) -> HttpResponse {
        std::string cookie = parseSessionCookie(req["Cookie"]);

        auto sess = gUserSessions.find(cookie);

        if (sess == gUserSessions.end())
            goto bad_session;
        
        return {200, templates::HomeTemplate(gUsers[sess->second].displayName)};

        bad_session:
        HttpResponse response{302};
        response["Location"] = "/";
        response["Set-Cookie"] = "tinyhttpChatSess=removed; Max-Age=-1; path=/; HttpOnly";
        return response;
    });

    s.when("/logout")->requested([](const HttpRequest& req) -> HttpResponse {
        std::string cookie = parseSessionCookie(req["Cookie"]);

        if (!cookie.empty())
            destroySession(cookie);

        HttpResponse response{302};
        response["Location"] = "/";
        response["Set-Cookie"] = "tinyhttpChatSess=removed; Max-Age=-1; path=/; HttpOnly";
        return response;
    });

    s.websocket("/ws")->handleWith<ChatSocketHandler>();

    s.startListening(8088);
    return 0;
}
