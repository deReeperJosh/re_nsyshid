#include "infinityendpoints.h"
#include "../devices/Infinity.h"

void registerInfinityEndpoints(HttpServer &server) {

    server.when("/device/infinity")->requested([](const HttpRequest &req) {
        miniJson::Json::_object ret;
        for (uint8_t i = 0; i < MAX_FIGURES; i++) {
            miniJson::Json::_object figure;
            uint32_t figNum = g_infinitybase.FindFigureFromSlot(i);
            if (figNum == 0) {
                figure["id"]   = 0;
                figure["name"] = "Empty";
            } else {
                const auto figureData = g_infinitybase.FindFigure(figNum);
                figure["position"]    = figureData.first;
                figure["name"]        = figureData.second;
                figure["id"]          = int(figNum);
            }
            ret[std::to_string(i)] = figure;
        }
        return HttpResponse{200, ret};
    });

    server.when("/device/infinity/remove")
            ->options([](const HttpRequest &req) {
                HttpResponse res(200);
                res["Access-Control-Allow-Methods"] = "POST, OPTIONS";
                res["Access-Control-Allow-Headers"] = "Content-Type";
                res["Access-Control-Max-Age"]       = "86400";
                return res;
            })
            ->posted([](const HttpRequest &req) {
                miniJson::Json::_object res;
                auto body = req.json();

                if (!body.isObject()) {
                    res["error"] = "INVALID_BODY";
                    return HttpResponse{200, res};
                }
                auto removeRequest  = body.toObject();
                const auto baseSlot = removeRequest["slot"];
                if (!baseSlot.isNumber()) {
                    res["error"] = "INVALID_SLOT_PARAM";
                    return HttpResponse{400, res};
                }
                uint8_t slot = uint8_t(baseSlot.toDouble());
                if (slot >= MAX_FIGURES || slot < 0) {
                    res["error"] = "INVALID_SLOT";
                    return HttpResponse{400, res};
                }
                if (g_infinitybase.RemoveFigure(slot)) {
                    res["message"] = "Figure removed";
                    return HttpResponse{200, res};
                } else {
                    res["message"] = "NO_FIGURE_IN_SLOT";
                    return HttpResponse{404, res};
                }
            });

    server.when("/device/infinity/load")
            ->options([](const HttpRequest &req) {
                HttpResponse res(200);
                res["Access-Control-Allow-Methods"] = "POST, OPTIONS";
                res["Access-Control-Allow-Headers"] = "Content-Type";
                res["Access-Control-Max-Age"]       = "86400";
                return res;
            })
            ->posted([](const HttpRequest &req) {
                miniJson::Json::_object res;
                auto body = req.json();

                if (!body.isObject()) {
                    res["error"] = "INVALID_BODY";
                    return HttpResponse{200, res};
                }
                auto loadRequest    = body.toObject();
                std::string file    = "/vol/external01/wiiu/re_nsyshid/" + loadRequest["file"].toString();
                const auto baseSlot = loadRequest["slot"];
                if (!baseSlot.isNumber()) {
                    res["error"] = "INVALID_SLOT_PARAM";
                    return HttpResponse{400, res};
                }
                uint8_t slot = uint8_t(baseSlot.toDouble());
                if (slot >= MAX_FIGURES || slot < 0) {
                    res["error"] = "INVALID_SLOT";
                    return HttpResponse{400, res};
                }
                std::array<uint8_t, INF_FIGURE_SIZE> fileData;
                int ret_code = FSUtils::ReadFromFile(file.c_str(), fileData.data(), fileData.size());
                if (ret_code == fileData.size()) {
                    if (g_infinitybase.LoadFigure(fileData, file, slot) == 0) {
                        res["error"] = "FAILED_TO_LOAD_FIGURE";
                        return HttpResponse{404, res};
                    }
                    res["message"] = "Figure loaded";
                    return HttpResponse{200, res};
                } else {
                    res["error"] = "FIGURE_FILE_TOO_SMALL";
                    return HttpResponse{400, res};
                }
            });
}
