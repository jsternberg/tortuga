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
    conf.env.CXXFLAGS = ['-std=c++11']

def build(bld):
    bld.stlib(target='tortuga',
              source='src/subsystem.cc',
              use='zeromq jsoncpp')
