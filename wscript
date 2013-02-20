#!/usr/bin/env python
# encoding: utf-8

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')
    conf.check_cfg(package='libzmq', args='--cflags --libs',
                   uselib_store='zeromq')
    conf.check_cfg(package='jsoncpp', args='--cflags --libs',
                   uselib_store='jsoncpp')

    # check for a c++11 compiler that uses the features we care about
    conf.env.CXXFLAGS = ['-std=c++11']
    conf.check_cxx(msg='Checking for c++11 compiler', fragment='''
#include <memory>
#include <thread>
#include <vector>
using namespace std;
unique_ptr<int> get_int() { return unique_ptr<int>(new int(5)); }
int main() {
  auto ptr = get_int();
  vector<int> arr{*ptr};
  for (auto& i : arr) {
    auto thread = unique_ptr<std::thread>(new std::thread([](int n) {}, i));
    thread->join();
  }
  return 0;
}
''')

def build(bld):
    bld.stlib(name='libtortuga',
              target='tortuga',
              source='src/subsystem.cc',
              use='zeromq jsoncpp')
    bld.program(target='tortuga',
                source='src/tortuga.cc src/fileop.cc',
                use='libtortuga')
    bld.program(target='eventhub',
                source='src/eventhub.cc',
                use='libtortuga')
