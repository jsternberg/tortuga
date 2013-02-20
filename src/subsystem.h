#pragma once

#include <zmq.hpp>
#include <json/json.h>
#include <memory>
#include <iostream>
#include <string>
#include <vector>

struct SubsystemConfig {
  const char *progname;
  std::string file;
};

struct Subsystem {
  Subsystem(int io_threads = 1);
  virtual ~Subsystem() {}

  /// Disable implicit copy constructor/assignment operator
  Subsystem(const Subsystem&) = delete;
  Subsystem& operator=(const Subsystem&) = delete;

  /// Setup code for the Subsystem
  /// Use this instead of a constructor
  virtual void setup(const Json::Value&) = 0;

  /// Subscribe to events from an address
  void subscribe(const char *addr);

  /// Handle events from publishers that were previously subscribed to
  virtual void handle(const Json::Value&) = 0;

  /// Bind as a publisher to an address
  void bind(const char *addr);

  /// Publish an event to all subscribers
  /// Bind must have been called before this function
  void publish(const Json::Value&);

  /// Parse command line options for this subsystem
  bool parse_options(int argc, char *argv[], SubsystemConfig& config);

  /// Print command line usage for the subsystem
  virtual void usage(const char *progname);

  /// Run the subsystem's main loop
  /// Make sure to call "setup" before this function
  int run();

  /// Main command line program for the subsystem
  int main(int argc, char *argv[]);

  /// Setup this subsystem to call an update function at a certain interval
  /// The run function will try to call "update" at that interval as closely
  /// as it can manage. If the subsystem is set to call update, this has a
  /// higher priority than processing incoming requests/events.
  void set_update_interval(int interval);

  /// Update function to be called at a certain interval.
  /// This is not pure virtual since subsystems that don't call
  /// set_update_interval don't need to define this.
  virtual void update() {}

private:
  zmq::context_t context_;
  std::unique_ptr<zmq::socket_t> subscribe_;
  std::unique_ptr<zmq::socket_t> publisher_;
  long interval_;
};

template <typename T>
int subsystem_main(int argc, char *argv[]) {
  std::unique_ptr<T> subsystem(new T);
  return subsystem->main(argc, argv);
}

#define SUBSYSTEM_MAIN(TYPE) \
  int main(int argc, char *argv[]) { return subsystem_main<TYPE>(argc, argv); }