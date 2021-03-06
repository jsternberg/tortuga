/*
 * Copyright (C) 2007 Robotics at Maryland
 * Copyright (C) 2007 Joseph Lisee <jlisee@umd.edu>
 * All rights reserved.
 *
 * Author: Joseph Lisee <jlisee@umd.edu>
 * File:  wrapper/vehicle/test/src/TestIVehicle.cxx
 */

// STD Includes
#include <cmath>

// Library Includes
#include <UnitTest++/UnitTest++.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>

// Project Includes
#include "vehicle/test/include/MockVehicle.h"
#include "vehicle/test/include/MockThruster.h"
#include "vehicle/test/include/MockPowerSource.h"
#include "vehicle/test/include/MockTempSensor.h"

namespace py = boost::python;

struct DeviceFixture
{
    DeviceFixture() :
        main_module(py::import("__main__")),
        main_namespace(main_module.attr("__dict__")),
        eval(boost::bind(py::exec, _1, main_namespace, main_namespace)),
        vehicle(new MockVehicle),
        vehiclePtr(boost::shared_ptr<ram::vehicle::IVehicle>())
    {
        main_namespace["vdev"] = py::import("ext.vehicle.device");

        // Add vehicle to the python environment
        vehiclePtr = boost::shared_ptr<ram::vehicle::IVehicle>(vehicle);
        main_namespace["vehicle"] = vehiclePtr;
    }

    py::object main_module;
    py::object main_namespace;
    boost::function<py::object (py::str)> eval;
    MockVehicle* vehicle;
    boost::shared_ptr<ram::vehicle::IVehicle> vehiclePtr;
};

TEST(DeviceImport)
{
    py::import("ext.vehicle.device");
}

TEST_FIXTURE(DeviceFixture, Vehicle)
{
    try {
        // Create MockThruster and add it to the MockVehicle
        MockThruster* thruster = new MockThruster("AftThruster");
        ram::vehicle::device::IThrusterPtr thrusterPtr(thruster);
        vehicle->devices[thruster->getName()] = thrusterPtr;

        // Test getForce
        thruster->force = 10;

        eval("thruster = vehicle.getDevice('AftThruster')\n"
             "force = thruster.getForce()");

        int force = py::extract<double>(main_namespace["force"]);
        CHECK_EQUAL(thruster->force, force);

        // Test set force
        eval("thruster.setForce(13)");
        CHECK_EQUAL(13, thruster->force);
    } catch(py::error_already_set err) { PyErr_Print(); throw err; }
}

TEST_FIXTURE(DeviceFixture, IPowerSource)
{
    try {
        // Create MockThruster and add it to the MockVehicle
        MockPowerSource* powerSource = new MockPowerSource("PowerSource");
        ram::vehicle::device::IPowerSourcePtr powerSourcePtr(powerSource);
        vehicle->devices[powerSource->getName()] = powerSourcePtr;

        // Test isEnabled
        powerSource->enabled = true;

        eval("powerSource = vehicle.getDevice('PowerSource')\n"
             "enabled = powerSource.isEnabled()");

        bool enabled = py::extract<double>(main_namespace["enabled"]);
        CHECK_EQUAL(powerSource->isEnabled(), enabled);

        // Test setEnabled
        //eval("powerSource.setEnabled(false)");
        //CHECK_EQUAL(false, powerSource->isEnabled());
    } catch(py::error_already_set err) { PyErr_Print(); throw err; }
}

TEST_FIXTURE(DeviceFixture, ITempSensor)
{
    try {
        // Create MockThruster and add it to the MockVehicle
        MockTempSensor* tempSensor = new MockTempSensor("TempSensor");
        ram::vehicle::device::ITempSensorPtr tempSensorPtr(tempSensor);
        vehicle->devices[tempSensor->getName()] = tempSensorPtr;

        // Test getTemp
        tempSensor->temp = 23;

        eval("tempSensor = vehicle.getDevice('TempSensor')\n"
             "temp = tempSensor.getTemp()");

        int temp = py::extract<double>(main_namespace["temp"]);
        CHECK_EQUAL(tempSensor->getTemp(), temp);
    } catch(py::error_already_set err) { PyErr_Print(); throw err; }
}

int main()
{
    // We must register this contverted so we can pass a pointer to python
    py::register_ptr_to_python< boost::shared_ptr<ram::vehicle::IVehicle> >();
    Py_Initialize();
    return UnitTest::RunAllTests();
}
