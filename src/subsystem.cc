
#include "subsystem.h"
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
using namespace std;

Subsystem::Subsystem(int io_threads) : context_(io_threads) {
  subscribe_ = unique_ptr<zmq::socket_t>(new zmq::socket_t(context_, ZMQ_SUB));
  subscribe_->setsockopt(ZMQ_SUBSCRIBE, "", 0);
}

void Subsystem::subscribe(const char *addr) {
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

static long difftime(struct timeval *tv1, struct timeval *tv2) {
    return 1000000*(tv1->tv_sec-tv2->tv_sec)+tv1->tv_usec-tv2->tv_usec;
}

int Subsystem::run() {
  struct timeval last, now;
  if (gettimeofday(&last, 0) != 0)
    return 1;

  for (;;) {
    zmq::pollitem_t pollfds[] = {
      { (void*)(*subscribe_), 0, ZMQ_POLLIN, 0 }
    };

    long timeout = -1;
    if (interval_ > 0) {
      if (gettimeofday(&now, 0) != 0)
        return 1;
      timeout = std::max<long>(interval_-difftime(&now, &last), 0);
      if (timeout <= 0)
        goto timeout;
    }

    if (zmq::poll(pollfds, 1, timeout) == 0)
      goto timeout;

    if (subscribe_ && pollfds[0].revents & ZMQ_POLLIN) {
      zmq::message_t message;
      subscribe_->recv(&message);

      string document((char*)message.data(), message.size());
      Json::Reader reader;
      Json::Value root;
      if (reader.parse(document, root, false)) {
        handle(root);
      }
    }
    continue;

  timeout:
    update();
    if (gettimeofday(&last, 0) != 0)
      return 1;
  }
  return 0;
}

void Subsystem::set_update_interval(int interval) {
  interval_ = 1000000/interval;
}
