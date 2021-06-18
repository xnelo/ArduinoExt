////////////////////////////////////////
// Written By: Spencer Hoffa
// File: ArduinoExtCommon.h
// Copyright: (c)2021
/// \copyright \link LICENSE.md MIT License\endlink 2021 Spencer Hoffa 
////////////////////////////////////////

#ifndef ARDUINOEXT_ARDUINOEXTCOMMON_H__2_24_2021
#define ARDUINOEXT_ARDUINOEXTCOMMON_H__2_24_2021

#include <Arduino.h>

namespace ArduinoExt
{
	// memoryFree header
	// From http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1213583720/15
	// ...written by user "mem".
	// https://github.com/mpflaga/Arduino-MemoryFree/blob/master/MemoryFree.h
	// accessed: 2/24/2021
	int freeMemory();

	inline uint16_t htons(uint16_t x)
	{
		return (((x) << 8) | (((x) >> 8) & 0xFF));
	}

	inline uint16_t ntohs(uint16_t x)
	{
		return htons(x);
	}
}

#endif //ARDUINOEXT_ARDUINOEXTCOMMON_H__2_24_2021
