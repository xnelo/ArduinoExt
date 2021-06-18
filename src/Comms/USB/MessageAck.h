////////////////////////////////////////
// \file MessageAck.h
// \author Spencer Hoffa
// \copyright \link LICENSE.md MIT License\endlink 2021 Spencer Hoffa 
////////////////////////////////////////
#ifndef ARDUINOEXT_COMMS_USB_MESSAGEACK_H__3_1_2021
#define ARDUINOEXT_COMMS_USB_MESSAGEACK_H__3_1_2021

#include <Arduino.h>

#include "../../ArduinoExtCommon.h"

#include "MessageBase.h"

namespace ArduinoExt
{
	namespace Comms
	{
		namespace USB
		{
			class MessageAck: public MessageBase
			{
			public:
				uint8_t TypeToAck;
				uint16_t MessageAckId;
				uint8_t Error;

				MessageAck():
					TypeToAck(static_cast<uint8_t>(MessageType::Unknown)),
					MessageAckId(0),
					Error(0)
				{}

				virtual MessageType MsgType() = 0;
				virtual uint8_t MsgTypeId() = 0;

				virtual uint8_t MessageSize()
				{
					return 4;
				}

				virtual bool RequireAck()
				{
					return false;
				}

				virtual size_t ToBuffer(uint8_t*buffer, uint8_t bufferSize, uint8_t start)
				{
					uint8_t len = MessageSize();
					if (buffer == NULL) return -1;
					if (start < 0) return -2;
					if (len+start > bufferSize) return -3;
					buffer[start] = TypeToAck;
					uint16_t netVal = ArduinoExt::htons(MessageAckId);
					memcpy(buffer + start + 1, &netVal, 2);
					buffer[start + 3] = Error;
					return len;
				}

				virtual size_t Parse(uint8_t* buffer, uint8_t bufferSize, uint8_t start)
				{
					return -1;
				}
			};

			//specializations
			class MessageAckFail: public MessageAck
			{
			public:
				virtual MessageType MsgType()
				{
					return MessageType::AckFail;
				}

				virtual uint8_t MsgTypeId()
				{
					return static_cast<uint8_t>(MessageType::AckFail);
				}
			};

			class MessageAckSuccess: public MessageAck
			{
			public:
				virtual MessageType MsgType()
				{
					return MessageType::AckSuccess;
				}

				virtual uint8_t MsgTypeId()
				{
					return static_cast<uint8_t>(MessageType::AckSuccess);
				}
			};
		}
	}
}
#endif //!ARDUINOEXT_COMMS_USB_MESSAGEACK_H__3_1_2021