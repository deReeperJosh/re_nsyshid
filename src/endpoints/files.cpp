#include "files.h"

#include <dirent.h>
#include <sstream>
#include <string>

std::string urlDecode(const std::string &urlEncodedString) {
    std::istringstream input(urlEncodedString);
    std::ostringstream output;

    char hex;
    int value;

    while (input >> std::noskipws >> hex) {
        if (hex == '%') {
            if (input >> std::hex >> value) {
                output << static_cast<char>(value);
            }
        } else {
            output << hex;
        }
    }

    return output.str();
}

void registerFileEndpoints(HttpServer &server) {
    server.when("/files")->requested([](const HttpRequest &req) {
        std::string path = "/vol/external01/wiiu/re_nsyshid/"; // Base directory
        if (!req.getQuery().empty() && req.getQuery().length() > 1) {
            if (req.getQuery().find("path=") == 1) {                             // Ensure query starts with 'path='
                path += urlDecode(req.getQuery().substr(req.getQuery().find("path=") + 5)); // Append query to path for subdirectory
                DEBUG_FUNCTION_LINE("Loading path %s", path.c_str());
            }
        }

        miniJson::Json::_object ret;
        miniJson::Json::_array fileList;

        struct dirent *ent;
        DIR *dir = opendir(path.c_str());
        if (dir) {
            for (int i = 0; i < 256 && (ent = readdir(dir)) != NULL; i++) {
                if (ent->d_type & DT_DIR) {
                    fileList.push_back(ent->d_name + std::string("/")); // Append '/' to indicate directory
                } else {
                    fileList.push_back(ent->d_name);
                }
            }
        } else {
            return HttpResponse{404, "text/plain", "Directory not found"};
        }
        ret["files"] = fileList;

        return HttpResponse{200, ret};
    });
}
