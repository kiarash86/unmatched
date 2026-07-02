#pragma once
#include<vector>
#include<string>
#include<nlohmann/json.hpp> 
#include<filesystem>
#include <fstream>

nlohmann::json load(const std::string & path);

 std::vector<std::string> listFiles(const std::string & folder);