
#include "fileop.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <json/json.h>
using namespace std;

static void fatal(const char* msg, ...) {
  va_list ap;
  fprintf(stderr, "tortuga: fatal: ");
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  exit(1);
}

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
    tortuga::join_mutable(prefix, progname);
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

void usage() {
  fprintf(stderr,
"usage: tortuga [options]\n"
"\n"
"options:\n"
"  -c, --config FILE  specify program configuration\n"
"  -h, --help         print this help message\n"
          );
}

static bool interrupted = false;
void set_interrupted_flag(int signum) {
  interrupted = true;
}

struct SubsystemConfig {
  string command;
  string stdin;
};

struct Tortuga;
struct Subprocess {
  Subprocess(const string& name);
  ~Subprocess();

  bool start(Tortuga *tortuga, const SubsystemConfig& command);
  void read_pipe();
  bool done() const { return fd_ == -1; }
  void finish();

  string name_;
  pid_t pid_;
  int fd_;

  // output buffer
  string buf_;
};

struct Tortuga {
  int main(int argc, char *argv[]);
  Subprocess *add(const string& name, const SubsystemConfig& config);
  int run();
  bool setup();
  void teardown();

  string libexec_;

  vector<Subprocess*> subprocs_;
  struct sigaction old_act_;
  sigset_t old_mask_;
};

Subprocess::Subprocess(const string& name)
  : name_(name), pid_(-1), fd_(-1) {}

Subprocess::~Subprocess() {
  if (fd_ >= 0)
    close(fd_);
  // Reap child if forgotten.
  if (pid_ != -1)
    finish();
}

void Subprocess::read_pipe() {
  char buf[4096];
  ssize_t len = read(fd_, buf, sizeof(buf));
  if (len > 0) {
    buf_.append(buf, len);
  } else {
    if (len < 0)
      fprintf(stderr, "%s: read: %s\n", name_.c_str(), strerror(errno));
    close(fd_);
    fd_ = -1;
  }

  // flush the output buffer on every newline
  for (;;) {
    size_t pos = buf_.find('\n');
    if (pos == string::npos)
      break;
    cout << name_ << ": ";
    cout.write(buf_.data(), pos+1);
    buf_.erase(0, pos+1);
  }

  if (done() && !buf_.empty()) {
    cout << name_ << ": " << buf_ << "\n";
    buf_.clear();
  }
}

bool Subprocess::start(Tortuga* tortuga, const SubsystemConfig& config) {
  int output_pipe[2];
  if (pipe(output_pipe) < 0) {
    fprintf(stderr, "%s: pipe: %s\n", name_.c_str(), strerror(errno));
    return false;
  }
  fd_ = output_pipe[0];

  int input_pipe[2];
  if (!config.stdin.empty()) {
    if (pipe(input_pipe) < 0) {
      fprintf(stderr, "%s: input pipe: %s\n", name_.c_str(), strerror(errno));
      return false;
    }
  }

  pid_ = fork();
  if (pid_ < 0) {
    fprintf(stderr, "%s: fork: %s\n", name_.c_str(), strerror(errno));
    return false;
  }

  if (pid_ == 0) {
    close(output_pipe[0]);

    // Track which fd we use to report errors on.
    int error_pipe = output_pipe[1];
    do {
      if (setpgid(0, 0) < 0)
        break;

      if (sigaction(SIGINT, &tortuga->old_act_, 0) < 0)
        break;
      if (sigprocmask(SIG_SETMASK, &tortuga->old_mask_, 0) < 0)
        break;

      if (config.stdin.empty()) {
        // Open /dev/null over stdin.
        input_pipe[0] = open("/dev/null", O_RDONLY);
      } else {
        close(input_pipe[1]);
      }

      if (input_pipe[0] < 0)
        break;
      if (dup2(input_pipe[0], 0) < 0)
        break;
      close(input_pipe[0]);

      if (dup2(output_pipe[1], 1) < 0 ||
          dup2(output_pipe[1], 2) < 0)
        break;

      // Now we can use stderr for errors.
      error_pipe = 2;
      close(output_pipe[1]);

      execl("/bin/sh", "/bin/sh", "-c", config.command.c_str(), (char *) NULL);
    } while (false);

    // If we get here, something went wrong; the execl should have
    // replaced us.
    char* err = strerror(errno);
    if (write(error_pipe, err, strlen(err)) < 0) {
      // If the write fails, there's nothing we can do.
      // But this block seems necessary to silence the warning.
    }
    _exit(1);
  }

  close(output_pipe[1]);
  if (!config.stdin.empty()) {
    close(input_pipe[0]);
    string stdin = config.stdin;
    do {
      ssize_t len = write(input_pipe[1], stdin.data(), min<size_t>(stdin.size(), 4096));
      stdin.erase(0, len);
    } while (!stdin.empty());
    close(input_pipe[1]);
  }
  return true;
}

void Subprocess::finish() {
  assert(pid_ != -1);
  int status;
  if (waitpid(pid_, &status, 0) < 0)
    fatal("waitpid(%d): %s", pid_, strerror(errno));
  pid_ = -1;
}

int Tortuga::main(int argc, char *argv[]) {
  const char *progname = argv[0];
  const option kLongOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "config", required_argument, NULL, 'c' },
    { NULL, 0, NULL, 0 }
  };

  const char *config_file = "tortuga.json";
  int opt;
  while ((opt = getopt_long(argc, argv, "c:h", kLongOptions, NULL)) != -1) {
    switch (opt) {
      case 'c':
        config_file = optarg;
        break;
      case 'h':
      default:
        usage();
        return 1;
    }
  }

  libexec_ = find_program_path(progname);
  tortuga::dirname(libexec_);
  if (!has_waf_lock(libexec_))
    tortuga::join_mutable(libexec_, "libexec");

  Json::Value root;
  {
    Json::Reader reader;
    ifstream in(config_file);
    if (!reader.parse(in, root, false)) {
      fprintf(stderr, "tortuga: could not parse config file %s\n", config_file);
      return 1;
    }
  }

  if (!setup())
    return 1;

  int status = 1;
  do {
    const Json::Value& config = root;
    for (const Json::Value& subsystem : config["subsystems"]) {
      if (!subsystem.isMember("name") || !subsystem["name"].isString()) {
        Json::StyledWriter writer;
        cout << "Subsystem definition requires a name\n"
             << writer.write(subsystem);
        break;
      }

      if (!subsystem.isMember("command") || !subsystem["command"].isString()) {
        Json::StyledWriter writer;
        cout << "Subsystem definition requires a command\n"
             << writer.write(subsystem);
        break;
      }

      string name = subsystem["name"].asString();
      SubsystemConfig subsystemConfig;
      subsystemConfig.command = subsystem["command"].asString();

      if (subsystem.isMember("config")) {
        Json::FastWriter writer;
        subsystemConfig.stdin = writer.write(subsystem["config"]);
      }
      add(name, subsystemConfig);
    }
    status = run();
  } while(false);
  teardown();
  return status;
}

Subprocess *Tortuga::add(const string& name, const SubsystemConfig& config) {
  Subprocess *subprocess = new Subprocess(name);
  if (!subprocess->start(this, config)) {
    delete subprocess;
    return nullptr;
  }
  subprocs_.push_back(subprocess);
  return subprocess;
}

int Tortuga::run() {
  for (;;) {
    if (subprocs_.empty())
      return 0;

    vector<pollfd> fds;
    nfds_t nfds = 0;

    for (Subprocess *proc : subprocs_) {
      int fd = proc->fd_;
      if (fd < 0)
        continue;
      pollfd pfd = { fd, POLLIN, 0 };
      fds.push_back(pfd);
      ++nfds;
    }

    int ret = ppoll(&fds.front(), nfds, NULL, &old_mask_);
    if (ret == -1) {
      if (errno != EINTR) {
        perror("tortuga: ppoll");
        return 1;
      }

      // Send interrupt signal to all child subprocesses
      for (Subprocess *proc : subprocs_) {
        kill(-proc->pid_, SIGINT);
      }
    }

    nfds_t cur_nfd = 0;
    for (vector<Subprocess*>::iterator i = subprocs_.begin();
         i != subprocs_.end(); ) {
      int fd = (*i)->fd_;
      if (fd < 0)
        continue;
      assert(fd == fds[cur_nfd].fd);
      if (fds[cur_nfd++].revents) {
        (*i)->read_pipe();
        if ((*i)->done()) {
          auto proc = *i;
          i = subprocs_.erase(i);
          proc->finish();
          delete proc;
          continue;
        }
      }
      ++i;
    }
  }
  return 0;
}

bool Tortuga::setup() {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  if (sigprocmask(SIG_BLOCK, &set, &old_mask_) < 0) {
    fprintf(stderr, "sigprocmask: %s", strerror(errno));
    return false;
  }

  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = set_interrupted_flag;
  if (sigaction(SIGINT, &act, &old_act_) < 0) {
    fprintf(stderr, "sigaction: %s", strerror(errno));
    return false;
  }
  return true;
}

void Tortuga::teardown() {
  // Interrupt all subprocesses
  for (Subprocess *proc : subprocs_) {
    kill(-proc->pid_, SIGINT);
    delete proc;
  }
  subprocs_.clear();

  // Not a whole lot we can do if these fail
  if (sigaction(SIGINT, &old_act_, 0) < 0)
    fatal("sigaction: %s", strerror(errno));
  if (sigprocmask(SIG_SETMASK, &old_mask_, 0) < 0)
    fatal("sigprocmask: %s", strerror(errno));
}

int main(int argc, char *argv[]) {
  auto tortuga = unique_ptr<Tortuga>(new Tortuga);
  return tortuga->main(argc, argv);
}
