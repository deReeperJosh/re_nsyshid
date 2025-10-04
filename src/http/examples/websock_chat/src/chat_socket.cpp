
#include "websock_chat.h"

std::set<ChatSocketHandler*> gChatClients;

void ChatSocketHandler::onConnect() {
    puts("Connect!");

    mSessionToken = parseSessionCookie((*mRequest)["Cookie"]);
    
    if (checkSession()) {
        mUser = &gUsers[gUserSessions[mSessionToken]];
        gChatClients.insert(this);
    }
}

void ChatSocketHandler::onTextMessage(const std::string& message) {
    if (!checkSession()) return;

    std::time_t now = std::time(0);

    std::string e;
    auto j = miniJson::Json::parse(message, e);

    if (!e.empty()) {
        sendJson(miniJson::Json::_object {
            { "type", "error" },
            { "error", e },
            { "time", (double) now }
        });

        return;
    }

    if (!j["message"].isString()) {
        sendJson(miniJson::Json::_object {
            { "type", "error" },
            { "error", "invalid message" },
            { "time", (double) now }
        });

        return;
    }

    std::string _message = j["message"].toString();

    if (_message.empty()) {
        sendJson(miniJson::Json::_object {
            { "type", "error" },
            { "error", "empty message" },
            { "time", (double) now }
        });

        return;
    }

    std::cout << " [" << mUser->displayName << "] -> " << _message << std::endl;

    sendJson(miniJson::Json::_object {
        { "type", "text_message" },
        { "sender", miniJson::Json::_object {
            { "id", "$self" },
            { "name", mUser->displayName }
        } },
        { "content", _message },
        { "time", (double) now }
    });

    auto toSend = miniJson::Json::_object {
        { "type", "text_message" },
        { "sender", miniJson::Json::_object {
            { "id", (double) mUser->id },
            { "name", mUser->displayName }
        } },
        { "content", _message },
        { "time", (double) now }
    };

    for (auto client : gChatClients)
        if (client != this)
            client->sendJson(toSend);
}

void ChatSocketHandler::onBinaryMessage(const std::vector<uint8_t>& data) {
    puts("Binary message!");
}

void ChatSocketHandler::onDisconnect() {
    puts("Disconnect!");
    gChatClients.erase(this);
}

bool ChatSocketHandler::checkSession() {
    auto sess = gUserSessions.find(mSessionToken);

    if (sess == gUserSessions.end()) {
        sendDisconnect();
        return false;
    }

    return true;
}
