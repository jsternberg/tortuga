/*
 * Copyright (C) 2009 Robotics at Maryland
 * Copyright (C) 2009 John Edmonds <pocketcookies2@gmail.com>
 * All rights reserved.
 *
 * Author: John Edmonds <pocketcookies2@gmail.com>
 * File:  packages/logging/include/Serialize.h"
 */


#ifndef RAM_LOGGING_SERIALIZE_H_03_05_2009
#define RAM_LOGGING_SERIALIZE_H_03_05_2009

// Library Includes
#include<boost/serialization/serialization.hpp>
#include<boost/serialization/split_free.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

// Project Includes
#include "core/include/Feature.h"
#include "core/include/EventPublisher.h"
#include "core/include/EventPublisherRegistry.h"
#include "core/include/Events.h"

#ifdef RAM_WITH_MATH
#include "math/include/Events.h"
#endif

#ifdef RAM_WITH_VISION
#include "vision/include/Events.h"
#endif

#ifdef RAM_WITH_VEHICLE
#include "vehicle/include/Events.h"
#endif

#ifdef RAM_WITH_CONTROL
#include "control/include/Events.h"
#endif

namespace ram {
namespace logging {

// ------------------------------------------------------------------------- //
//                     T Y P E   R E G I S T R A T I O N                     //
// ------------------------------------------------------------------------- //

template <class Archive>
void registerTypes(Archive& ar)
{
    // Core Events
    ar.register_type(static_cast<ram::core::StringEvent*>(NULL));

#ifdef RAM_WITH_MATH
    // Math Events
    ar.register_type(static_cast<ram::math::OrientationEvent*>(NULL));
    ar.register_type(static_cast<ram::math::Vector3Event*>(NULL));
    ar.register_type(static_cast<ram::math::NumericEvent*>(NULL));
#endif // RAM_WITH_MATH

#ifdef RAM_WITH_VISION    
    // Vision Events
    ar.register_type(static_cast<ram::vision::ImageEvent*>(NULL));
    ar.register_type(static_cast<ram::vision::RedLightEvent*>(NULL));
    ar.register_type(static_cast<ram::vision::PipeEvent*>(NULL));
    ar.register_type(static_cast<ram::vision::BinEvent*>(NULL));
    ar.register_type(static_cast<ram::vision::DuctEvent*>(NULL));
    ar.register_type(static_cast<ram::vision::SafeEvent*>(NULL));
    ar.register_type(static_cast<ram::vision::TargetEvent*>(NULL));
    ar.register_type(static_cast<ram::vision::BarbedWireEvent*>(NULL));
#endif // RAM_WITH_VISION

#ifdef RAM_WITH_VEHICLE    
    // Vehicle Events
    ar.register_type(static_cast<ram::vehicle::PowerSourceEvent*>(NULL));
    ar.register_type(static_cast<ram::vehicle::TempSensorEvent*>(NULL));
    ar.register_type(static_cast<ram::vehicle::ThrusterEvent*>(NULL));
    ar.register_type(static_cast<ram::vehicle::SonarEvent*>(NULL));
#endif // RAM_WITH_VEHICLE

#ifdef RAM_WITH_CONTROL    
    // Control Events
    ar.register_type(static_cast<ram::control::ParamSetupEvent*>(NULL));
    ar.register_type(static_cast<ram::control::ParamUpdateEvent*>(NULL));
#endif // RAM_WITH_CONTROL    
}
    
    
} // logging
} // ram


namespace boost {
namespace serialization {

// ------------------------------------------------------------------------- //
//                          B A S E   C L A S S E S                          //
// ------------------------------------------------------------------------- //

    
template<class Archive>
void save(Archive& ar, const ram::core::Event& t, unsigned int version)
{
    ar & t.type;
    ar & t.timeStamp;

    // Lookup the name of the sending publisher
    std::string name = "UNNAMED";
    if (t.sender)
        name = t.sender->getPublisherName();
    ar & name;
}
    
template<class Archive>
void load(Archive& ar, ram::core::Event& t, unsigned int version)
{
    ar & t.type;
    ar & t.timeStamp;

    // Read back the name of the sending publisher and assign back to an
    // existing publishing if possible
    std::string name;
    ar & name;
    t.sender = ram::core::EventPublisherRegistry::lookupByName(name);
}

// This wraps up the use of the different save and load functionality
template<class Archive>
void serialize(Archive & ar, ram::core::Event& t,
               const unsigned int file_version)
{
    split_free(ar, t, file_version); 
}
    
BOOST_SERIALIZATION_SHARED_PTR(ram::core::Event)

// ------------------------------------------------------------------------- //
//                           C O R E   E V E N T S                           //
// ------------------------------------------------------------------------- //

template <class Archive>
void serialize(Archive &ar, ram::core::StringEvent &t,
               const unsigned int file_version)
{ 
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.string;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::core::StringEvent)    


// ------------------------------------------------------------------------- //
//                           M A T H   E V E N T S                           //
// ------------------------------------------------------------------------- //

#ifdef RAM_WITH_MATH

template <class Archive>
void serialize(Archive &ar, ram::math::Vector3 &t,
               const unsigned int file_version)
{ 
  ar & t.x;
  ar & t.y;
  ar & t.z;
}


template <class Archive>
void serialize(Archive &ar, ram::math::Quaternion &t,
               const unsigned int file_version)
{ 
  ar & t.x;
  ar & t.y;
  ar & t.z;
  ar & t.w;
}


template <class Archive>
void serialize(Archive &ar, ram::math::OrientationEvent &t,
               const unsigned int file_version)
{ 
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.orientation;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::math::OrientationEvent)    


template <class Archive>
void serialize(Archive &ar, ram::math::Vector3Event &t,
               const unsigned int file_version)
{ 
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.vector3;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::math::Vector3Event)    


template <class Archive>
void serialize(Archive &ar, ram::math::NumericEvent &t,
               const unsigned int file_version)
{ 
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.number;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::math::NumericEvent)    

#endif // RAM_WITH_MATH

// ------------------------------------------------------------------------- //
//                         V I S I O N   E V E N T S                         //
// ------------------------------------------------------------------------- //

#ifdef RAM_WITH_VISION

// The image event is not stored currently
template<class Archive>
void save(Archive& ar, const ram::vision::ImageEvent& t, unsigned int version)
{
    ar & t.type;
    ar & t.timeStamp;
}
    
template<class Archive>
void load(Archive& ar, ram::vision::ImageEvent& t, unsigned int version)
{
    ar & t.type;
    ar & t.timeStamp;
    t.sender = 0;
}

template<class Archive>
void serialize(Archive & ar, ram::vision::ImageEvent& t,
               const unsigned int file_version)
{
    split_free(ar, t, file_version); 
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vision::ImageEvent)
    
    
template <class Archive>
void serialize(Archive &ar, ram::vision::RedLightEvent &t,
               const unsigned int file_version)
{ 
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.y;
  ar & (*((double*)(&t.azimuth)));
  ar & (*((double*)(&t.elevation)));
  ar & t.range;
  ar & t.x;
  ar & t.y;
  ar & t.pixCount;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vision::RedLightEvent)


template <class Archive>
void serialize(Archive &ar, ram::vision::PipeEvent &t,
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.id;
  ar & t.x;
  ar & t.y;
  ar & (*((double*)(&t.angle)));
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vision::PipeEvent)


template <class Archive>
void serialize(Archive &ar, ram::vision::BinEvent &t, 
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.y;
  ar & t.y;
  ar & t.id;
  ar & t.symbol;
  ar & (*((double*)(&t.angle)));
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vision::BinEvent)


template <class Archive>
void serialize(Archive &ar, ram::vision::DuctEvent &t, 
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.y;
  ar & t.x;
  ar & t.range;
  ar & t.alignment;
  ar & t.aligned;
  ar & t.visible;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vision::DuctEvent)


template <class Archive>
void serialize(Archive &ar, ram::vision::SafeEvent &t, 
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.x;
  ar & t.y;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vision::SafeEvent)


template <class Archive>
void serialize(Archive &ar, ram::vision::TargetEvent &t, 
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.x;
  ar & t.y;
  ar & t.squareNess;
  ar & t.range;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vision::TargetEvent)


template <class Archive>
void serialize(Archive &ar, ram::vision::BarbedWireEvent &t, 
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.topX;
  ar & t.topY;
  ar & t.topWidth;
  ar & t.bottomX;
  ar & t.bottomY;
  ar & t.bottomWidth;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vision::BarbedWireEvent)

#endif // RAM_WITH_VISION

// ------------------------------------------------------------------------- //
//                        V E H I C L E   E V E N T S                        //
// ------------------------------------------------------------------------- //

#ifdef RAM_WITH_VEHICLE

template <class Archive>
void serialize(Archive &ar, ram::vehicle::PowerSourceEvent &t, 
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.id;
  ar & t.enabled;
  ar & t.inUse;
  ar & t.voltage;
  ar & t.current;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vehicle::PowerSourceEvent)


template <class Archive>
void serialize(Archive &ar, ram::vehicle::TempSensorEvent &t, 
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.id;
  ar & t.temp;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vehicle::TempSensorEvent)


template <class Archive>
void serialize(Archive &ar, ram::vehicle::ThrusterEvent &t, 
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.address;
  ar & t.current;
  ar & t.enabled;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vehicle::ThrusterEvent)


template <class Archive>
void serialize(Archive &ar, ram::vehicle::SonarEvent &t, 
               const unsigned int file_version)
{
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.direction;
  ar & t.range;
  ar & t.pingTimeSec;
  ar & t.pingTimeUSec;
  ar & t.pingCount;
  ar & t.pingerID;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::vehicle::SonarEvent)

#endif // RAM_WITH_VEHICLE

// ------------------------------------------------------------------------- //
//                        C O N T R O L   E V E N T S                        //
// ------------------------------------------------------------------------- //

#ifdef RAM_WITH_CONTROL

template <class Archive>
void serialize(Archive &ar, ram::control::ParamSetupEvent &t,
               const unsigned int file_version)
{ 
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.labels;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::control::ParamSetupEvent)


template <class Archive>
void serialize(Archive &ar, ram::control::ParamUpdateEvent &t,
               const unsigned int file_version)
{ 
  ar & boost::serialization::base_object<ram::core::Event>(t);
  ar & t.values;
}

BOOST_SERIALIZATION_SHARED_PTR(ram::control::ParamUpdateEvent)

#endif // RAM_WITH_CONTROL

} // serialization
} // boost

#endif // RAM_LOGGING_SERIALIZE_H_03_05_2009
