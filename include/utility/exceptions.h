#pragma once
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>


class AppException : public std::runtime_error {
public:
  explicit AppException(const std::string &message) : std::runtime_error(message) {}
};


class FileException : public AppException { // cant open?
public:
  explicit FileException(const std::string &message) : AppException(message) {}
};


class JsonParseException : public AppException { // wrong form of json
public:
  explicit JsonParseException(const std::string &message) : AppException(message) {}
};

class DataFormatException : public AppException { // wanted int send string? bad thing to do
public:
  explicit DataFormatException(const std::string &message) : AppException(message) {}
};

class FactoryException : public AppException { // problem when creating them?
public:
  explicit FactoryException(const std::string &message) : AppException(message) {}
};


namespace json_util { // use this for checking json. better readibilty nothing more

inline const nlohmann::json &requireField(const nlohmann::json &j, const std::string &key,
                                           const std::string &context) {
  if (!j.contains(key)) {
    throw DataFormatException(context + ": missing required field '" + key + "'"); // no data for this field?
  }
  return j.at(key);
}

inline int requireInt(const nlohmann::json &j, const std::string &key,
                       const std::string &context) {
  const auto &field = requireField(j, key, context);
  if (!field.is_number()) {
    throw DataFormatException(context + ": field '" + key + "' must be a number"); // string to int? or similiar?
  }
  return field.get<int>();
}

inline std::string requireString(const nlohmann::json &j, const std::string &key,
                                  const std::string &context) {
  const auto &field = requireField(j, key, context);
  if (!field.is_string()) {
    throw DataFormatException(context + ": field '" + key + "' must be a string");
  }
  return field.get<std::string>();
}

} ;
