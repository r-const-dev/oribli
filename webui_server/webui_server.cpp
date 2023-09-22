#include "webui_server.h"

#include <httplib.h>

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <iostream>

extern std::map<std::string, const char*> kWebuiFiles;

namespace oribli {

const char* GetMimeType(const std::string& extension) {
  static std::map<std::string, const char*> kMimeTypes = {
      {".js", "text/javascript"},
      {".html", "text/html"}};
  static const char* kDefaultMimeType = "text/html";
  auto it = kMimeTypes.find(extension);
  if (it == kMimeTypes.end()) {
    return kDefaultMimeType;
  }
  return it->second;
}

void ServeWebui(httplib::Server* http_server, std::optional<std::string> webui_dir) {
  if (webui_dir.has_value()) {
    std::cout << "Serving webui from " << webui_dir.value() << std::endl;
    http_server->set_mount_point("/webui", webui_dir.value());
  } else if (!kWebuiFiles.empty()) {
    http_server->Get("/webui/:file", [](const httplib::Request& request,
                                        httplib::Response& response) {
      std::string file = request.path_params.at("file");
      std::map<std::string, const char*>::const_iterator fileIt = kWebuiFiles.find(file);
      if (fileIt == kWebuiFiles.end()) {
        response.status = 404;
        return;
      }
      std::string extension = std::filesystem::path(fileIt->first).extension();
      response.set_content(fileIt->second, GetMimeType(extension));
    });
  }
}

}  // namespace oribli