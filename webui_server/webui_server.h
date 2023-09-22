#pragma once

#include <optional>
#include <string>

namespace httplib {
class Server;
}  // namespace httplib

namespace oribli {
void ServeWebui(httplib::Server* server, std::optional<std::string> webui_dir);
}  // namespace oribli
