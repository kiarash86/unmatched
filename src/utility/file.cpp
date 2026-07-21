#include "../../include/utility/file.h"
#include <iostream>

nlohmann::json load(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "[load] Could not open file: " << path << std::endl;
    return nlohmann::json{};
  }

  try {
  
    return nlohmann::json::parse(file,  nullptr,
                                  true,
                                  true);
  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "[load] Failed to parse JSON file: " << path << " -> "
               << e.what() << std::endl;
    return nlohmann::json{};
  }
}

std::vector<std::string> listFiles(const std::string &folder) {
  std::vector<std::string> files;

  for (const auto &file : std::filesystem::directory_iterator(folder)) {
    if (file.is_regular_file()) {
      files.push_back(file.path().string());
    }
  }

  return files;
}