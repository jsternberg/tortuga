
#include "subsystem.h"
#include <json/json.h>

struct EventHub : Subsystem {
  void setup(const Json::Value&) {}

  void handle(const Json::Value& value) {
    publish(value);
  }
};

SUBSYSTEM_MAIN(EventHub)
