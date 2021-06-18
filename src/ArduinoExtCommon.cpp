////////////////////////////////////////
// Written By: Spencer Hoffa
// File: ArduinoExtCommon.cpp
// Copyright: (c)2021
/// \copyright \link LICENSE.md MIT License\endlink 2021 Spencer Hoffa 
////////////////////////////////////////

#include "ArduinoExtCommon.h"

// https://github.com/mpflaga/Arduino-MemoryFree/blob/master/MemoryFree.cpp
// Accessed on: 2/24/2021
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int ArduinoExt::freeMemory() { //NOTE: Spencer HOffa Added namespace
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^