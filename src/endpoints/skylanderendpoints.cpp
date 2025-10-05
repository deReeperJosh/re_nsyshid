#include "skylanderendpoints.h"
#include "../devices/Skylander.h"

void registerSkylanderEndpoints(HttpServer &server) {

    server.when("/device/skylander")->requested([](const HttpRequest &req) {
        miniJson::Json::_object ret;
        for (uint8_t i = 0; i < MAX_SKYLANDERS; i++) {
            miniJson::Json::_object skylander;
            const auto idvar       = g_skyportal.GetSkylanderIdFromUISlot(i);
            const auto skyName     = g_skyportal.GetSkylanderFromUISlot(i);
            skylander["id"]        = idvar.first;
            skylander["var"]       = idvar.second;
            skylander["name"]      = skyName;
            ret[std::to_string(i)] = skylander;
        }
        return HttpResponse{200, ret};
    });

    server.when("/device/skylander/remove")->posted([](const HttpRequest &req) {
        miniJson::Json::_object res;
        auto body = req.json();

        if (!body.isObject()) {
            res["error"] = "INVALID_BODY";
            return HttpResponse{200, res};
        }
        auto removeRequest    = body.toObject();
        const auto portalSlot = removeRequest["slot"];
        if (!portalSlot.isNumber()) {
            res["error"] = "INVALID_SLOT_PARAM";
            return HttpResponse{400, res};
        }
        uint8_t slot = uint8_t(portalSlot.toDouble());
        if (slot >= MAX_SKYLANDERS || slot < 0) {
            res["error"] = "INVALID_SLOT";
            return HttpResponse{400, res};
        }
        if (g_skyportal.RemoveSkylander(slot)) {
            res["message"] = "Skylander removed";
            return HttpResponse{200, res};
        } else {
            res["message"] = "NO_SKYLANDER_IN_SLOT";
            return HttpResponse{404, res};
        }
    });

    server.when("/device/skylander/load")->posted([](const HttpRequest &req) {
        miniJson::Json::_object res;
        auto body = req.json();

        if (!body.isObject()) {
            res["error"] = "INVALID_BODY";
            return HttpResponse{200, res};
        }
        auto loadRequest      = body.toObject();
        std::string file      = loadRequest["file"].toString();
        const auto portalSlot = loadRequest["slot"];
        if (!portalSlot.isNumber()) {
            res["error"] = "INVALID_SLOT_PARAM";
            return HttpResponse{400, res};
        }
        uint8_t slot = uint8_t(portalSlot.toDouble());
        if (slot >= MAX_SKYLANDERS || slot < 0) {
            res["error"] = "INVALID_SLOT";
            return HttpResponse{400, res};
        }
        std::array<uint8_t, 0x10 * 0x40> fileData;
        int ret_code = FSUtils::ReadFromFile(file.c_str(), fileData.data(), fileData.size());
        if (ret_code == fileData.size()) {
            if (!g_skyportal.LoadSkylander(fileData.data(), file, slot)) {
                res["error"] = "FAILED_TO_LOAD_SKYLANDER";
                return HttpResponse{404, res};
            }
            res["message"] = "Skylander loaded";
            return HttpResponse{200, res};
        } else {
            res["error"] = "SKYLANDER_FILE_TOO_SMALL";
            return HttpResponse{400, res};
        }
    });
}