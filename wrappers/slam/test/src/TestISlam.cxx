/*
 * Copyright (C) 2009 Robotics at Maryland
 * Copyright (C) 2009 Jonathan Sternberg <jsternbe@umd.edu>
 * All rights reserved.
 *
 * Author: Jonathan Sternberg <jsternbe@umd.edu>
 * File:  packages/slam/test/src/TestISlam.cxx
 */

// Library Includes
#include <UnitTest++/UnitTest++.h>

// Library Includes
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>

// Project Includes
#include "slam/include/ISlam.h"
#include "math/include/Vector2.h"

class MockSlam : public ram::slam::ISlam
{
public:
    MockSlam(std::string name) : ISlam(name) {}

    virtual ram::math::Vector2 getObjectPosition(std::string name) {
	return ram::math::Vector2(0, 0);
    };

    virtual ram::math::Vector2 getRelativePosition(std::string name) {
	return ram::math::Vector2(0, 0);
    };

    virtual bool hasObject(std::string) {
	return true;
    };
   
    virtual void setPriority(ram::core::IUpdatable::Priority) {};
    virtual ram::core::IUpdatable::Priority getPriority() {
        return ram::core::IUpdatable::NORMAL_PRIORITY;
    };
    virtual void setAffinity(size_t) {};
    virtual int getAffinity() {
        return -1;
    }
    virtual void update(double) {};
    virtual void background(int) {};
    virtual void unbackground(bool) {};
    virtual bool backgrounded() { return false; }
};
/**
TEST(MockController)
{
    MockController* mockController = new MockController("Controller");
    ram::control::IController* controller = mockController;

    controller->setSpeed(5);
    CHECK_EQUAL(5, mockController->speed);

    delete mockController;
}
**/

namespace py = boost::python;
/**
TEST(Emedding)
{
    try {
    py::object main_module = py::import("__main__");
    py::object main_namespace = main_module.attr("__dict__");
    py::object ignored = py::exec("result = 5 ** 2", main_namespace,
                                  main_namespace);
    int five_squared = py::extract<int>(main_namespace["result"]);

    CHECK_EQUAL(25, five_squared);
    } catch(py::error_already_set err) { PyErr_Print(); throw err; }
}

TEST(ControlImport)
{
    try {
    py::import("ext.control");
    } catch(py::error_already_set err) { PyErr_Print(); throw err; }
}

TEST(ControllerWrapping)
{
    try {
    
    // Create MockController and wrap it in smart point to allow python to
    // handle it
    MockController* mockController = new MockController("Controller");
    boost::shared_ptr<ram::control::IController> controllerPtr(mockController);

    // Create out namespace for the python code to operate in
    py::object main_module = py::import("__main__");
    py::object main_namespace = main_module.attr("__dict__");

    // Import our module to test and inject the controller object
    main_namespace["control"] = py::import("ext.control");
    main_namespace["controller"] = controllerPtr;

    // Create simple eval function
    boost::function<py::object (py::str)> eval =
        boost::bind(py::exec, _1, main_namespace, main_namespace);

    // Test speed (if this works everything else should to)
    eval("controller.setSpeed(3)");
    CHECK_EQUAL(3, mockController->speed);
    
    eval("speed = controller.getSpeed()");
    double speed = py::extract<double>(main_namespace["speed"]);
    CHECK_EQUAL(3, speed);
    
    } catch(py::error_already_set err) { PyErr_Print(); throw err; }
}
**/
int main()
{
    Py_Initialize();
    // We must register this converted so we can pass a pointer to python
    py::register_ptr_to_python< boost::shared_ptr<ram::slam::ISlam> >();
    return UnitTest::RunAllTests();
}