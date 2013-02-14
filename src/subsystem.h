#pragma once

#include <zmq.hpp>
#include <json/json.h>
#include <memory>
#include <string>
#include <vector>

struct Subsystem {
  Subsystem(int io_threads = 1);
  virtual ~Subsystem() {}
  Subsystem(const Subsystem&) = delete;
  Subsystem& operator=(const Subsystem&) = delete;

  virtual void setup() = 0;
  void subscribe(const char *addr);
  virtual void handle(const Json::Value&) = 0;
  void bind(const char *addr);
  void publish(const Json::Value&);
  int run();
  void set_update_interval(int interval);
  virtual void update() {}

  zmq::context_t context_;
  std::unique_ptr<zmq::socket_t> subscribe_;
  std::unique_ptr<zmq::socket_t> publisher_;
  long interval_;
};
