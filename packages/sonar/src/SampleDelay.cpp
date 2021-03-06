/**
 * @file SampleDelay.cpp
 *
 * @author Leo Singer
 * @author Copyright 2008 Robotics@Maryland. All rights reserved.
 *
 */


#include "SampleDelay.h"


#include <string.h>
#include <assert.h>


namespace ram {
namespace sonar {


SampleDelay::SampleDelay(int nchannels, int numSamples)
{
	assert(numSamples >= 0);
	buflen = nchannels * (numSamples + 1);
	buf = new adcdata_t[buflen];
	bufend = &buf[buflen];
	bufptr = buf;
	increment = nchannels;
	purge();
}


SampleDelay::~SampleDelay()
{
	delete [] buf;
}


void SampleDelay::writeSample(adcdata_t *sample)
{
	memcpy(bufptr, sample, increment * sizeof(adcdata_t));
	bufptr += increment;
	if (bufptr >= bufend)
		bufptr = buf;
}


adcdata_t * SampleDelay::readSample()
{
	return bufptr;
}


void SampleDelay::purge()
{
	bzero(buf, buflen * sizeof(adcdata_t));
}


} // namespace sonar
} // namespace ram
