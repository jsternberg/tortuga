/*
 * Copyright (C) 2007 Robotics at Maryland
 * Copyright (C) 2007 Joseph Lisee <jlisee@umd.edu>
 * All rights reserved.
 *
 * Author: Joseph Lisee <jlisee@umd.edu>
 * File:  packages/vision/test/src/UnitTestMain.cxx
 */

// Library Includes
#include <UnitTest++/UnitTest++.h>

// Project Includes
#include "vision/test/include/detail/UnitTestChecksImp.h"
#include "math/test/include/detail/MathChecksImp.h"

int main()
{
    return UnitTest::RunAllTests();
}
