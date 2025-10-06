#include "dimensionsendpoints.h"
#include "../devices/Dimensions.h"

static uint8_t derivePadFromIndex(uint8_t index) {
    switch (index) {
        case 0:
        case 3:
        case 4:
            return 2;

        case 2:
        case 5:
        case 6:
            return 3;

        case 1:
            return 1;


        default:
            return 0;
    }
}

void registerDimensionsEndpoints(HttpServer &server) {

    server.when("/device/dimensions")->requested([](const HttpRequest &req) {
        miniJson::Json::_object ret;
        std::array<std::optional<uint32_t>, 7> currentFigures = g_dimensionstoypad.GetCurrentFigures();
        for (uint8_t i = 0; i < 7; i++) {
            miniJson::Json::_object figure;
            if (currentFigures[i]) {
                figure["id"]   = int(currentFigures[i].value());
                figure["name"] = g_dimensionstoypad.FindFigure(currentFigures[i].value());
            } else {
                figure["id"]   = 0;
                figure["name"] = "None";
            }
            ret[std::to_string(i)] = figure;
        }
        return HttpResponse{200, ret};
    });

    server.when("/device/dimensions/remove")
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
                auto removeRequest = body.toObject();
                const auto padSlot = removeRequest["slot"];
                if (!padSlot.isNumber()) {
                    res["error"] = "INVALID_SLOT_PARAM";
                    return HttpResponse{400, res};
                }
                uint8_t slot = uint8_t(padSlot.toDouble());
                if (slot >= 7 || slot < 0) {
                    res["error"] = "INVALID_SLOT";
                    return HttpResponse{400, res};
                }
                uint8_t pad = derivePadFromIndex(slot);
                if (g_dimensionstoypad.RemoveFigure(pad, slot, true)) {
                    res["message"] = "Figure removed";
                    return HttpResponse{200, res};
                } else {
                    res["message"] = "NO_FIGURE_IN_SLOT";
                    return HttpResponse{404, res};
                }
            });

    server.when("/device/dimensions/load")
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
                    return HttpResponse{400, res};
                }
                auto loadRequest   = body.toObject();
                std::string file   = "/vol/external01/wiiu/re_nsyshid/" + loadRequest["file"].toString();
                const auto padSlot = loadRequest["slot"];
                if (!padSlot.isNumber()) {
                    res["error"] = "INVALID_SLOT_PARAM";
                    return HttpResponse{400, res};
                }
                uint8_t slot = uint8_t(padSlot.toDouble());
                if (slot >= 7 || slot < 0) {
                    res["error"] = "INVALID_SLOT";
                    return HttpResponse{400, res};
                }
                uint8_t pad = derivePadFromIndex(slot);
                if (file.empty()) {
                    res["error"] = "MISSING_FILE_PARAM";
                    return HttpResponse{400, res};
                }
                std::array<uint8_t, DIM_FIGURE_SIZE> buf;
                int result = FSUtils::ReadFromFile(file.c_str(), buf.data(), buf.size());
                if (result != DIM_FIGURE_SIZE) {
                    res["error"] = "COULD_NOT_READ_FILE";
                    return HttpResponse{400, res};
                }
                uint32_t id = g_dimensionstoypad.LoadFigure(buf, file, pad, slot);
                if (id == 0) {
                    res["error"] = "FAILED_TO_LOAD_FIGURE";
                    return HttpResponse{400, res};
                } else {
                    res["message"]   = "Figure loaded";
                    res["figure_id"] = int(id);
                    return HttpResponse{200, res};
                }
            });

    server.when("/device/dimensions/move")
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
                    return HttpResponse{400, res};
                }
                auto moveRequest   = body.toObject();
                const auto oldSlot = moveRequest["oldSlot"];
                const auto newSlot = moveRequest["newSlot"];
                if (!oldSlot.isNumber() || !newSlot.isNumber()) {
                    res["error"] = "INVALID_SLOT_PARAM";
                    return HttpResponse{400, res};
                }
                uint8_t oldSlotValue = uint8_t(oldSlot.toDouble());
                uint8_t newSlotValue = uint8_t(newSlot.toDouble());
                if (oldSlotValue >= 7 || oldSlotValue < 0 || newSlotValue >= 7 || newSlotValue < 0) {
                    res["error"] = "INVALID_SLOT";
                    return HttpResponse{400, res};
                }
                uint8_t oldPad = derivePadFromIndex(oldSlotValue);
                uint8_t newPad = derivePadFromIndex(newSlotValue);
                if (g_dimensionstoypad.MoveFigure(newPad, newSlotValue, oldPad, oldSlotValue)) {
                    res["message"] = "Figure moved";
                    return HttpResponse{200, res};
                } else {
                    res["message"] = "NO_FIGURE_IN_OLD_SLOT_OR_NEW_SLOT_OCCUPIED";
                    return HttpResponse{400, res};
                }
            });

    server.when("/device/dimensions/create")
            ->options([](const HttpRequest &req) {
                HttpResponse res(200);
                res["Access-Control-Allow-Methods"] = "POST, OPTIONS";
                res["Access-Control-Allow-Headers"] = "Content-Type";
                res["Access-Control-Max-Age"]       = "86400";
                return res;
            })
            ->posted([](const HttpRequest &req) {
                miniJson::Json::_object res;
                auto body           = req.json();
                auto createRequest  = body.toObject();
                const auto figureId = createRequest["id"];
                if (!figureId.isNumber()) {
                    res["error"] = "INVALID_ID_PARAM";
                    return HttpResponse{400, res};
                }
                uint16_t id      = uint16_t(figureId.toDouble());
                std::string name = g_dimensionstoypad.FindFigure(id);
                if (g_dimensionstoypad.CreateFigure("/vol/external01/wiiu/re_nsyshid/Dimensions/" + name + ".bin", id)) {
                    res["message"] = "Figure created";
                    res["file"]    = "/Dimensions/" + name + ".bin";
                    return HttpResponse{200, res};
                } else {
                    res["error"] = "FAILED_TO_CREATE_FIGURE";
                    return HttpResponse{400, res};
                }
                return HttpResponse{404, res};
            });
}