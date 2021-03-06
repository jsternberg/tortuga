/*
 * Copyright (C) 2009 Robotics at Maryland
 * Copyright (C) 2009 Joseph Lisee <jlisee@umd.edu>
 * All rights reserved.
 *
 * Author: Joseph Lisee <jlisee@umd.edu>
 * File:  packages/vision/include/device/LoopStateEstimator.h
 */

#ifndef RAM_VEHICLE_DEVICE_LOOPSTATEESTIMATOR_06_26_2009
#define RAM_VEHICLE_DEVICE_LOOPSTATEESTIMATOR_06_26_2009

// STD Includes
#include <map>

// Project Includes
#include "vehicle/include/device/IStateEstimator.h"
#include "vehicle/include/device/Device.h"

#include "core/include/ConfigNode.h"
#include "core/include/ReadWriteMutex.h"

// Must Be Included last
#include "vehicle/include/Export.h"

namespace ram {
namespace vehicle {
namespace device {

/** Simple loop back estimator, ie. it doesn't esimator anything */
class RAM_EXPORT LoopStateEstimator :
        public Device, // for getName
        public IStateEstimator
        // boost::noncopyable
{
public:
    LoopStateEstimator(core::ConfigNode config,
                       core::EventHubPtr eventHub = core::EventHubPtr(),
                       IVehiclePtr vehicle = IVehiclePtr());
    
    virtual ~LoopStateEstimator();

    virtual int orientationUpdate(math::Quaternion orientation,
				   double timeStamp);

    virtual int velocityUpdate(math::Vector2 velocity,
				double timeStamp);

    virtual int positionUpdate(math::Vector2 position,
				double timeStamp);
    
    virtual int depthUpdate(double depth,
			     double timeStamp);
    
    virtual math::Quaternion getOrientation(std::string obj = "vehicle");

    virtual math::Vector2 getVelocity(std::string obj = "vehicle");

    virtual math::Vector2 getPosition(std::string obj = "vehicle");
    
    virtual double getDepth(std::string obj = "vehicle");

    virtual bool hasObject(std::string obj);

    // Device Options
    virtual std::string getName() { return Device::getName(); }
    
    virtual void update(double timestep) {}

    virtual void setPriority(core::IUpdatable::Priority) {}

    virtual core::IUpdatable::Priority getPriority() {
        return IUpdatable::NORMAL_PRIORITY;
    }

    virtual void setAffinity(size_t) {};
    
    virtual int getAffinity() {
        return -1;
    };
    
    virtual void background(int interval) {
        //Updatable::background(interval);
    };
    
    virtual void unbackground(bool join = false) {
        //Updatable::unbackground(join);
    };
    virtual bool backgrounded() {
        return true;
        //return Updatable::backgrounded();
    };

    
protected:
    core::ReadWriteMutex m_mutex;

    math::Quaternion m_orientation;
    math::Vector2 m_velocity;
    math::Vector2 m_position;
    double m_depth;

    core::ReadWriteMutex m_objMutex;
    std::map<std::string, math::Quaternion> m_objOrientation;
    std::map<std::string, math::Vector2> m_objPosition;
    std::map<std::string, math::Vector2> m_objVelocity;
    std::map<std::string, double> m_objDepth;
};
    
} // namespace device
} // namespace vehicle
} // namespace ram

#endif // RAM_VEHICLE_DEVICE_LOOPSTATEESTIMATOR_06_26_2009
