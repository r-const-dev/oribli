#include <iostream>

void ShowUsage(const char* binary_name) {
  std::cout << "Usage: " << binary_name << " <command> <args>" << std::endl;
}

void CommandError(const char* error) {
  std::cerr << "Error: " << error << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    CommandError("Missing command");
    ShowUsage(argv[0]);
    return -1;
  }
  std::string command = argv[1];
  return 0;
}
