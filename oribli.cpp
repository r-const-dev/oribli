#include <cstring>
#include <string>
#include <sstream>
#include <stdexcept>
#include <errno.h>
#include <iostream>
#include <filesystem>
#include <fstream>

void ShowUsage(const char* binary_name) {
  std::cout << "Usage: " << binary_name << " <command> <args>" << std::endl;
}

void CommandError(const char* error) {
  std::cerr << "Error: " << error << std::endl;
}

void ExecCmd(const std::string& cmd) {
  std::cerr << cmd << std::endl;
  FILE* cmd_out = popen(cmd.c_str(), "r");
  if (!cmd_out) {
    throw std::runtime_error(std::string("Command [") +  cmd + "] failed. Error: " + std::strerror(errno));
  }
  char line[8192];
  while (fgets(line, sizeof(line), cmd_out) != NULL) {
    std::cout << line << std::endl;
  }
  int cmd_exit_code = pclose(cmd_out);
  if (cmd_exit_code != 0) {
    throw std::runtime_error(std::string("Command [") + cmd + "] finished with error exit code: " + std::to_string(cmd_exit_code));
  }
}

const char* kVcpkgPath = "/opt/vcpkg";
const char* kVcpkgBin = "vcpkg";
const char* kVcpkgToolChain = "scripts/buildsystems/vcpkg.cmake";
const char* kVcpkgInstalled = "vcpkg_installed";

std::filesystem::path VcpkgPath() {
  const char* homedir = getenv("HOME");
  if (!homedir) {
    return kVcpkgPath;
  }
  std::filesystem::path config = std::filesystem::path(homedir) / ".config/oribli";
  if (!std::filesystem::exists(config)) {
    return kVcpkgPath;
  }
  std::ifstream metadata(config);
  std::string line;
  while (std::getline(metadata, line)) {
    if (*std::mismatch(line.begin(), line.end(), "VCPKG_PATH").second == '\0') {
      int colon_pos = line.find(':');
      if (colon_pos == std::string::npos) {
        continue;
      }
      std::string value = line.substr(colon_pos+1);
      int start = value.find_first_not_of(" \t");
      if (start) {
        value = value.substr(start);
      }
      int end = value.find_last_not_of(" \t");
      if (end != value.size() - 1) {
        value = value.substr(0, end);
      }
      return value;
    }
  }
  return kVcpkgPath;
}

int VcpkgDeps(int deps_count, const char** deps) {
  std::ofstream vcpkg_json("vcpkg.json");
  vcpkg_json << "{" << std::endl;
  vcpkg_json << "  \"name\": " << std::filesystem::current_path().filename() << "," << std::endl;
  vcpkg_json << "  \"dependencies\": " << "[" << std::endl;
  for (int i = 0; i < deps_count; ++i) {
    vcpkg_json << "    \"" << deps[i] << "\"";
    if (i != deps_count - 1) {
      vcpkg_json << ",";
    }
    vcpkg_json << std::endl;
  }
  vcpkg_json << "  ]" << std::endl;
  vcpkg_json << "}" << std::endl;
  vcpkg_json.close();
  
  std::stringstream cmd;
  cmd << VcpkgPath() / kVcpkgBin;
  cmd << " " << "install";
  ExecCmd(cmd.str());
  return 0;
}

int CMake(int args_count, const char** args)  {
  // RUN cmake -DCMAKE_BUILD_TYPE=release -DVCPKG_INSTALLED_DIR=/build/oridupes/vcpkg_installed/  /oridupes/src/
  std::stringstream cmd;
  cmd << "cmake";
  cmd << " " << "-DCMAKE_BUILD_TYPE=release";
  cmd << " " << "-DVCPKG_INSTALLED_DIR=" << std::filesystem::current_path() / kVcpkgInstalled;
  cmd << " " << "-DCMAKE_TOOLCHAIN_FILE=" << VcpkgPath() / kVcpkgToolChain;
  cmd << " " << "-DORIBLI_CMAKE=/usr/lib/cmake/oribli/oribli.cmake";
  for (int i = 0; i < args_count; ++i) {
    cmd << " " << args[i];
  }
  
  ExecCmd(cmd.str());
  return 0;
}

int main(int argc, const char** argv) {
  if (argc < 2) {
    CommandError("Missing command");
    ShowUsage(argv[0]);
    return -1;
  }
  std::string command = argv[1];
  if (command == "cmake") {
    if (argc < 3) {
      CommandError("cmake command requires at least the source argument");
      ShowUsage(argv[0]);
      return -1;
    }
    return CMake(argc - 2, argv + 2);
  }
  if (command == "deps") {
    if (argc < 3) {
      CommandError("At least one dependency expected.");
      ShowUsage(argv[0]);
      return -1;
    }
    return VcpkgDeps(argc - 2, argv + 2);
  }
  return 0;
}
