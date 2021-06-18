////////////////////////////////////////
// \file MessageTypeEnum.h
// \author Spencer Hoffa
// \copyright \link LICENSE.md MIT License\endlink 2021 Spencer Hoffa 
////////////////////////////////////////
#ifndef ARDUINOEXT_COMMS_USB_MESSAGETYPEENUM_H__3_1_2021
#define ARDUINOEXT_COMMS_USB_MESSAGETYPEENUM_H__3_1_2021

#include <Arduino.h>

namespace ArduinoExt
{
	namespace Comms
	{
		namespace USB
		{
			enum class MessageType: uint8_t
			{
				Unknown = 0,
				AckSuccess = 1,
				AckFail = 2,
				String = 3,
				MESSAGE_TYPE_MAX = 63
			};
		}
	}
}

#endif //!ARDUINOEXT_COMMS_USB_MESSAGETYPEENUM_H__3_1_2021