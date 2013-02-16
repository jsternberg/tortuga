#pragma once

#include <string>

namespace tortuga {
  std::string getcwd();
  std::string abspath(const std::string& path);
  void normpath(std::string& path);
  bool file_exists(const std::string& path);
  void dirname(std::string& path);
  void join_mutable(std::string& path, const std::string& next);
  std::string join(const std::string& path, const std::string& next);
}
