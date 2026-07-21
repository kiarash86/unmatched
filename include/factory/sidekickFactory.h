#pragma once
#include <memory>
#include "../model/sidekick.h"
#include <nlohmann/json.hpp>
#include "../utility/file.h"

class SidekickFactory {
public:
  static std::unique_ptr<Sidekick> create(const std::string &path);
};
