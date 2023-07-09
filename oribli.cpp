#include <cstring>
#include <string>
#include <sstream>
#include <stdexcept>
#include <errno.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

bool StartsWith(const char* s, const char* prefix) {
  while (*s == *prefix) {
    ++s;
    ++prefix;
  }
  return (*prefix == '\0');
}

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

bool ParseStringFlag(int* pargc, char*** pargv, const char* flag, std::string* value) {
  int& argc = *pargc;
  char**& argv = *pargv;
  std::vector<int> args_to_remove;
  bool capture_value =  false;
  bool has_value = false;
  for (int i = 2; i < argc; ++i) {
    if (capture_value) {
      value->assign(argv[i]);
      capture_value = false;
      has_value = true;
      args_to_remove.push_back(i);
      continue;
    }
    if (StartsWith(argv[i], flag)) {
      args_to_remove.push_back(i);
      const char* pvalue = argv[i] + strlen(flag);
      if (*pvalue == '=' || *pvalue == ':') {
        ++pvalue;
        value->assign(pvalue);
        has_value = !value->empty();
      } else {
        capture_value = true;
      }
      continue;
    }
  }
  if (!has_value) {
    return false;
  }
  while (!args_to_remove.empty()) {
    for (int i = args_to_remove.back(); i < argc - 1; ++i) {
      argv[i] = argv[i+1];
    }
    --argc;
    args_to_remove.pop_back();
  }
  return true;
}

int Embed(const std::string& hdr_path, const std::string& src_path, const std::string& map_name, int files_count, const char** files) {
  std::ofstream hdr(hdr_path);
  hdr << "#pragma once" << std::endl;
  hdr << "#include <map>" << std::endl;
  hdr << "#include <string>" << std::endl;
  hdr << "extern std::map<std::string, const char*> " << map_name << ";" << std::endl;
  hdr.close();

  std::ofstream src(src_path);
  src << "#include <map>" << std::endl;
  src << "#include <string>" << std::endl;
  src << "namespace {" << std::endl;
  src << "  std::map<std::string, const char*> CreateMap() {" << std::endl;
  src << "    std::map<std::string, const char*> m;" << std::endl;
  for (int i = 0; i < files_count; ++i) {
    std::ifstream in(files[i]);
    src << std::endl;
    src << std::endl;
    src <<  "    m[" << std::filesystem::path(files[i]).filename() << "] = R\"file(" << std::endl;
    src << in.rdbuf();
    src << "test";
    src << "    )file\";" << std::endl;
    in.close();
  }
  src << std::endl;
  src << std::endl;
  src << "    return m;" << std::endl;
  src << "  }" << std::endl;
  src << "} // namespace" << std::endl;
  src << std::endl;
  src << "std::map<std::string, const char*> " << map_name << " = CreateMap();" << std::endl;
  src.close();

  return 0;
}

int main(int argc, char** argv) {
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
    return CMake(argc - 2, (const char**)(argv + 2));
  }
  if (command == "deps") {
    if (argc < 3) {
      CommandError("At least one dependency expected.");
      ShowUsage(argv[0]);
      return -1;
    }
    return VcpkgDeps(argc - 2, (const char**)(argv + 2));
  }
  if (command == "embed") {
    std::string hdr;
    if (!ParseStringFlag(&argc, &argv, "--hdr", &hdr)) {
      CommandError("embed requires --hdr=<output-header-file-name>");
    }
    std::string src;
    if (!ParseStringFlag(&argc, &argv, "--src", &src)) {
      CommandError("embed requires --src=<output-source-file-name>");
    }
    std::string map_name;
    if (!ParseStringFlag(&argc, &argv, "--map", &map_name)) {
      CommandError("embed requires --map=<output-constant-map-name>");
    }
    return Embed(hdr, src, map_name, argc - 2, (const char**)(argv + 2));
  }
  return 0;
}
