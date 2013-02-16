#pragma once

#include <string>

namespace tortuga {
  std::string getcwd();
  std::string abspath(const std::string& path);
  void normpath(std::string& path);
  bool file_exists(const std::string& path);
  void dirname(std::string& path);
  void join(std::string& path, const std::string& next);
}
