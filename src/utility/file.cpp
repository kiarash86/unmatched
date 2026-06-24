#include "../../include/utility/file.h"
nlohmann::json load(const std::string &path) {
  std::ifstream file(path);
  nlohmann::json js;
  file >> js;
  return js;
}

std::vector<std::string> listFiles(const std::string &folder) {
  std::vector<std::string> files;

  for (const auto &file : fs::directory_iterator(folder)) {
    if (file.is_regular_file()) {
      files.push_back(file.path().string());
    }
  }

  return files;
}