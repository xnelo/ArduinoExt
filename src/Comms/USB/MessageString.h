////////////////////////////////////////
// \file MessageString.h
// \author Spencer Hoffa
// \copyright \link LICENSE.md MIT License\endlink 2021 Spencer Hoffa 
////////////////////////////////////////
#ifndef ARDUINOEXT_COMMS_USB_MESSAGESTRING_H__3_1_2021
#define ARDUINOEXT_COMMS_USB_MESSAGESTRING_H__3_1_2021

#include <Arduino.h>
#include "MessageBase.h"

namespace ArduinoExt
{
	namespace Comms
	{
		namespace USB
		{
			class MessageString: public MessageBase
			{
			private:
				const char* m_MessageString;
			public:
				MessageString(const char* msg):
					m_MessageString(msg)
				{}

				virtual MessageType MsgType()
				{
					return MessageType::String;
				}

				virtual uint8_t MsgTypeId()
				{
					return static_cast<uint8_t>(MessageType::String);
				}

				virtual uint8_t MessageSize()
				{
					return strlen(m_MessageString);
				}

				virtual bool RequireAck()
				{
					return false;
				}

				virtual size_t ToBuffer(uint8_t*buffer, uint8_t bufferSize, uint8_t start)
				{
					size_t len = strlen(m_MessageString);
					if (buffer == NULL) return -1;
					if (start < 0) return -2;
					if (len+start > bufferSize) return -3;
					memcpy(buffer + start, m_MessageString, len);
					return len;
				}

				virtual size_t Parse(uint8_t* buffer, uint8_t bufferSize, uint8_t start)
				{
					return -1;
				}
			};

			class MessageStringF: public MessageBase
			{
			private:
				const __FlashStringHelper* m_MessageString;
			public:
				MessageStringF(const __FlashStringHelper* msg):
					m_MessageString(msg)
				{}

				virtual MessageType MsgType()
				{
					return MessageType::String;
				}

				virtual uint8_t MsgTypeId()
				{
					return static_cast<uint8_t>(MessageType::String);
				}

				virtual uint8_t MessageSize()
				{
					PGM_P msp = reinterpret_cast<PGM_P>(m_MessageString);
					return strlen_P(msp);
				}

				virtual bool RequireAck()
				{
					return false;
				}

				virtual size_t ToBuffer(uint8_t*buffer, uint8_t bufferSize, uint8_t start)
				{
					PGM_P msp = reinterpret_cast<PGM_P>(m_MessageString);
					size_t len = strlen_P(msp);
					if (buffer == NULL) return -1;
					if (start < 0) return -2;
					if (len+start > bufferSize) return -3;
					memcpy_P(buffer + start, msp, len);
					return len;
				}

				virtual size_t Parse(uint8_t* buffer, uint8_t bufferSize, uint8_t start)
				{
					return -1;
				}
			};
		}
	}
}

#endif //!ARDUINOEXT_COMMS_USB_MESSAGESTRING_H__3_1_2021