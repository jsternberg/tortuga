
#include "subsystem.h"
#include <iostream>
using namespace std;

void Subsystem::subscribe(const char *addr) {
  if (!subscribe_) {
    subscribe_ = unique_ptr<zmq::socket_t>(new zmq::socket_t(context_, ZMQ_SUB));
    subscribe_->setsockopt(ZMQ_SUBSCRIBE, "", 0);
  }
  subscribe_->connect(addr);
}

void Subsystem::bind(const char *addr) {
  if (!publisher_)
    publisher_ = unique_ptr<zmq::socket_t>(new zmq::socket_t(context_, ZMQ_PUB));
  publisher_->bind(addr);
}

void Subsystem::publish(const Json::Value& value) {
  assert(publisher_ != nullptr);
  if (!publisher_)
    return;

  Json::FastWriter writer;
  string document = writer.write(value);
  zmq::message_t message(document.size());
  memcpy(message.data(), document.data(), document.size());
  publisher_->send(message);
}

static int add_poll_socket(vector<zmq::pollitem_t>& pollfds,
                           unique_ptr<zmq::socket_t>& socket, short int type) {
  if (socket) {
    zmq::pollitem_t fd = { (void*)(*socket), 0, type, 0 };
    pollfds.push_back(fd);
  }
  return pollfds.size()-1;
}

int Subsystem::run() {
  for (;;) {
    vector<zmq::pollitem_t> pollfds;
    int subscribe_fd = add_poll_socket(pollfds, subscribe_, ZMQ_POLLIN);
    if (!pollfds.empty()) {
      zmq::poll(&pollfds.front(), pollfds.size());

      if (subscribe_ && pollfds[subscribe_fd].revents & ZMQ_POLLIN) {
        zmq::message_t message;
        subscribe_->recv(&message);

        string document((char*)message.data(), message.size());
        Json::Reader reader;
        Json::Value root;
        if (reader.parse(document, root, false)) {
          handle(root);
        }
      }
    }
  }
  return 0;
}
