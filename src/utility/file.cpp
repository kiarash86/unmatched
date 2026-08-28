#include "utility/file.h"
#include "utility/exceptions.h"
#include <iostream>

nlohmann::json load(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw FileException("Could not open file: " + path);
  }

  try {
    return nlohmann::json::parse(file, nullptr, true, true);
  } catch (const nlohmann::json::parse_error &e) {
    throw JsonParseException("Failed to parse JSON file: " + path + " -> " + e.what());
  }
}

std::vector<std::string> listFiles(const std::string &folder) {
  std::vector<std::string> files;

  try {
    for (const auto &file : std::filesystem::directory_iterator(folder)) {
      if (file.is_regular_file()) {
        files.push_back(file.path().string());
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    throw FileException("Could not list folder: " + folder + " -> " + e.what());
  }

  return files;
}
