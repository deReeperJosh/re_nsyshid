#include "status.h"

void registerStatusEndpoints(HttpServer &server) {

    server.when("/status")->requested([](const HttpRequest &req) {
        EmulationStatus status;
        DeviceToEmulate deviceToEmulate;
        WUPSStorageAPI::Get("emulationStatus", status);
        WUPSStorageAPI::Get("emulatedDevice", deviceToEmulate);
        miniJson::Json::_object ret;
        ret["status"] = status;
        ret["device"] = deviceToEmulate;
        return HttpResponse{200, ret};
    });

    server.when("/status/enable")->posted([](const HttpRequest &req) {
        miniJson::Json::_object ret;
        EmulationStatus status;
        WUPSStorageAPI::Get("emulationStatus", status);
        if (status == DISABLED) {
            WUPSStorageAPI::Store("emulationStatus", ENABLED);
        }
        ret["status"] = "enabled";
        return HttpResponse{200, ret};
    });

    server.when("/status/disable")->posted([](const HttpRequest &req) {
        miniJson::Json::_object ret;
        EmulationStatus status;
        WUPSStorageAPI::Get("emulationStatus", status);
        if (status == ENABLED) {
            WUPSStorageAPI::Store("emulationStatus", DISABLED);
        }
        ret["status"] = "disabled";
        return HttpResponse{200, ret};
    });

    server.when("/status/device")->posted([](const HttpRequest &req) {
        miniJson::Json::_object ret;
        if (req.content().empty()) {
            ret["error"] = "NO_DEVICE_SPECIFIED";
            return HttpResponse{400, ret};
        }
        std::string deviceStr           = req.content();
        DeviceToEmulate deviceToEmulate = NONE;
        if (deviceStr == "skylander") {
            deviceToEmulate = SKYLANDER;
        } else if (deviceStr == "infinity") {
            deviceToEmulate = INFINITY;
        } else if (deviceStr == "dimensions") {
            deviceToEmulate = DIMENSIONS;
        } else if (deviceStr == "none") {
            deviceToEmulate = NONE;
        } else {
            ret["error"] = "INVALID_DEVICE_SPECIFIED";
            return HttpResponse{400, ret};
        }
        WUPSStorageAPI::Store("emulatedDevice", deviceToEmulate);
        ret["status"] = "Device set";
        return HttpResponse{200, ret};
    });
}
