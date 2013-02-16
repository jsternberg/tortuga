
#include "fileop.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <iostream>
using namespace std;

static string find_path(const string& progname) {
  // Try to find the program name on that path environment
  char *path = getenv("PATH");
  if (!path)
    return string();

  // Path seems to be empty...
  if (!*path)
    return string();

  for (;;) {
    char *next = strchrnul(path, ':');
    string prefix(path, next-path);
    tortuga::join(prefix, progname);
    if (tortuga::file_exists(prefix))
      return prefix;

    if (!*next)
      return string();
    path = next+1;
  }
}

static string find_program_path(const char *progname) {
  string path = progname;
  if (path.find('/') == string::npos)
    path.assign(find_path(progname));
  path.assign(tortuga::abspath(path));
  tortuga::normpath(path);
  return path;
}

static bool has_waf_lock(const string& path) {
  DIR *dir = opendir(path.c_str());
  struct dirent *dirent;
  bool found = false;
  while ((dirent = readdir(dir)) != NULL) {
    string name = dirent->d_name;
    if (name.size() >= 9 && memcmp(name.data(), ".lock-waf", 9) == 0) {
      found = true;
      break;
    }
  }
  closedir(dir);
  return found;
}

static string find_libexec(const char *progname) {
  string path = find_program_path(progname);
  tortuga::dirname(path);
  if (!has_waf_lock(path))
    tortuga::join(path, "libexec");
  return path;
}

int main(int argc, char *argv[]) {
  cout << find_libexec(argv[0]) << endl;
  return 0;
}
