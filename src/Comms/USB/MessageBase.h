////////////////////////////////////////
// \file MessageBase.h
// \author Spencer Hoffa
// \copyright \link LICENSE.md MIT License\endlink 2021 Spencer Hoffa 
////////////////////////////////////////
#ifndef ARDUINOEXT_COMMS_USB_MESSAGEBASE_H__3_1_2021
#define ARDUINOEXT_COMMS_USB_MESSAGEBASE_H__3_1_2021

#include <Arduino.h>
#include "MessageTypeEnum.h"

namespace ArduinoExt
{
	namespace Comms
	{
		namespace USB
		{
			class MessageBase
			{
			public:
				MessageBase():
					AckUponReceive(false),
					MessageId(0)
				{ }
				bool AckUponReceive;
				uint16_t MessageId;

				/// \brief Get the Message type id number.
				///
				/// We cannot use the MessageTypeEnum here because we need to allow user
				/// defined message types to be parsed.
				/// \return Return the messagetype id number 0-3 are reserved for this library.
				virtual uint8_t MsgTypeId() = 0;

				/// \brief Get the size of the message.
				virtual uint8_t MessageSize() = 0;

				/// \brief Does this message require acknowledgment
				virtual bool RequireAck() = 0;

				/// \brief Parse the data in the buffer into this instance of the message.
				virtual size_t Parse(uint8_t* buffer, uint8_t bufferSize, uint8_t start) = 0;

				/// \brief Put the data from this instance of the message into the passed in buffer.
				virtual size_t ToBuffer(uint8_t*buffer, uint8_t bufferSize, uint8_t start) = 0;
			};
		}
	}
}
#endif //!ARDUINOEXT_COMMS_USB_MESSAGEBASE_H__3_1_2021