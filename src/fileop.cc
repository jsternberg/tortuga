
#include "fileop.h"
#include <vector>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
using namespace std;

namespace tortuga {
  string getcwd() {
    string cwd(64, '\0');
    while (::getcwd(const_cast<char*>(cwd.data()), cwd.size()) == NULL) {
      cwd.resize(cwd.size() << 1);
    }
    cwd.resize(strlen(cwd.data()));
    return cwd;
  }

  string abspath(const string& path) {
    if (path.empty())
      return string();
    if (path[0] == '/')
      return path;
    return getcwd() + "/" + path;
  }

  void normpath(string& path) {
    if (path.empty())
      return;

    size_t pos = 0;
    bool abspath = path[0] == '/';
    if (abspath)
      ++pos;

    vector<string> components;
    while (pos < path.size()) {
      size_t delim = path.find('/', pos);
      if (delim == pos) {
        ++pos;
        continue;
      } else if (delim == string::npos) {
        delim = path.size();
      }

      string p(&path[pos], delim-pos);
      if (p == ".") {
        p.clear();
      } else if (p == "..") {
        if (!components.empty()) {
          components.pop_back();
          p.clear();
        }
      }

      if (!p.empty())
        components.push_back(p);

      pos = delim+1;
    }

    path.clear();
    for (const string& p : components) {
      if (abspath)
        path.append(1, '/');
      path.append(p);
      abspath = true;
    }
  }

  bool file_exists(const string& path) {
    struct stat buf;
    return stat(path.c_str(), &buf) == 0;
  }

  void dirname(string& path) {
    size_t pos = path.rfind('/');
    if (pos == string::npos)
      pos = 0;
    path.resize(pos);
  }

  void join(string& path, const string& next) {
    path.append(1, '/');
    path.append(next);
  }
}
