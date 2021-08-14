////////////////////////////////////////
// \file ArduinoUsb.h
// \author Spencer Hoffa
// \copyright \link LICENSE.md MIT License\endlink 2021 Spencer Hoffa 
////////////////////////////////////////
#ifndef ARDUINOEXT_COMMS_USB_ARDUINOUSB_H__6_16_2021
#define ARDUINOEXT_COMMS_USB_ARDUINOUSB_H__6_16_2021

#include <Arduino.h>
#include "../../Data/Buffer.h"
#include "../../ArduinoExtCommon.h"
#include "../../Math/Crc16.h"

#include "MessageTypeEnum.h"

namespace ArduinoExt
{
	namespace Comms
	{
		namespace USB
		{
			class ArduinoUsb
			{
			public:
				// Header organization
				// Byte Num | size | data in it
				//     0    |   1  | The size of message (header + n) = (4 + n)
				//     1    |   1  | Msg Type
				//     2    |   2  | Checksum
				//     4    |   n  | data/payload
				static const uint8_t MSG_SIZE_INDEX = 0;
				static const uint8_t MSG_SIZE_SIZE = 1;

				static const uint8_t MSG_TYPE_INDEX = 1;
				static const uint8_t MSG_TYPE_SIZE = 1;

				static const uint8_t CKSUM_INDEX = 2;
				static const uint8_t CKSUM_SIZE = 2;

				static const uint8_t DATA_INDEX = 4;

				static const uint8_t HEADER_SIZE = MSG_SIZE_SIZE + MSG_TYPE_SIZE + CKSUM_SIZE; // 4 bytes

				static const uint8_t MAX_BUFFER_SIZE = 64;
				static const uint16_t RECEIVED_BUFFER_SIZE = MAX_BUFFER_SIZE * 4;
			private:
				uint8_t m_Buffer[MAX_BUFFER_SIZE];
				uint8_t m_ReceivedArray[RECEIVED_BUFFER_SIZE];
				ArduinoExt::Data::Buffer<uint8_t> m_ReceivedBuffer;
			public:
				/// \brief Constructor
				ArduinoUsb():
					m_Buffer(),
					m_ReceivedArray(),
					m_ReceivedBuffer(m_ReceivedArray, RECEIVED_BUFFER_SIZE)
				{}

				/// \brief Get the next message that we have received over usb connection.
				/// \param [out]buffer The buffer the message will be stored in.
				/// \param [in]bufferSize The size of the buffer to store data in
				/// \return The amount of data put into the buffer.
				size_t GetNextMessage(uint8_t* buffer, uint8_t bufferSize)
				{
					if (buffer == NULL)
					{
						return -1;
					}

					char msg[32];

					while (m_ReceivedBuffer.HasData())
					{
						uint8_t msgSize = m_ReceivedBuffer.Peek(0);
						if (msgSize <= 0)
						{
							// remove this byte.. there is a problem
							m_ReceivedBuffer.Remove(1);
						}
						else if (!m_ReceivedBuffer.HasData(msgSize))
						{
							return 0;
						}
						else if (msgSize - HEADER_SIZE > bufferSize)
						{
							//error
							sprintf(msg, "passed in buffer too small.");
							SendDebug(msg);
							return -2;
						}
						else
						{
							size_t retVal = 0;
							// we have enough data
							// get the message
							uint8_t* rawData = m_ReceivedBuffer.GetData();

							// validate checksum
							uint16_t cksum;
							memcpy(&cksum, rawData + CKSUM_INDEX, CKSUM_SIZE);
							cksum = ArduinoExt::ntohs(cksum);
							memset(rawData + CKSUM_INDEX, 0, CKSUM_SIZE);

							uint16_t check_cksum = Math::Crc16::XModemCrc(rawData, 0, msgSize);
							if (check_cksum != cksum)
							{
								sprintf(msg, "Cksum mismatch %d != %d", check_cksum, cksum);
								SendDebug(msg);
							}
							else
							{
								MessageType mt = static_cast<MessageType>(rawData[MSG_TYPE_INDEX]);
								if (mt != MessageType::Data)
								{
									//error
									sprintf(msg, "Message type error '%d'", static_cast<uint8_t>(mt));
									SendDebug(msg);
								}
								else
								{
									memcpy(buffer, rawData + DATA_INDEX, msgSize - HEADER_SIZE);
									retVal = msgSize - HEADER_SIZE;
								}
							}
							m_ReceivedBuffer.Remove(msgSize);
							return retVal;
						}
					}
					return 0;
				}

				/// \brief Receive data from the usb connection and place it in the buffer.
				void ReceiveData()
				{
					if (Serial.available() > 0)
					{
						size_t amtReceived = Serial.readBytes(m_Buffer, MAX_BUFFER_SIZE);
						if (amtReceived > 0)
						{
							m_ReceivedBuffer.AddData(m_Buffer, amtReceived);
						}
					}
				}

				/// \brief Send data across the usb connection
				/// This will add the appropriate header before sending it on the wire. A checksum value will be added 
				/// to ensure the data gets to the destination intact.
				/// \param data Pointer to an array of data to send.
				/// \param amt The amount of data from the array to send
				/// \return The amount of data sent on the wire.
				size_t Send(uint8_t* data, uint8_t amt)
				{
					return Send(data, amt, MessageType::Data);
				}

				size_t SendDebug(const char* debugString)
				{
					return Send((uint8_t*)debugString, (uint8_t)strlen(debugString), MessageType::Debug);
				}

			private:
				/// \brief Send data across the usb connection
				/// This will add the appropriate header before sending it on the wire. A checksum value will be added 
				/// to ensure the data gets to the destination intact.
				/// \param data Pointer to an array of data to send.
				/// \param amt The amount of data from the array to send
				/// \param msgType The type of data sent
				/// \return The amount of data sent on the wire.
				size_t Send(uint8_t* data, uint8_t amt, MessageType msgType)
				{
					// Header organization
					// Byte Num | size | data in it
					//     0    |   1  | The size of message
					//     1    |   1  | Msg Type
					//     2    |   2  | Checksum
					//     4    |   n  | data/payload
					if (data == NULL) return -1;
					uint8_t msgSize = amt + HEADER_SIZE;
					if (Serial.availableForWrite() < msgSize) return -2;
					m_Buffer[MSG_SIZE_INDEX] = msgSize;
					m_Buffer[MSG_TYPE_INDEX] = static_cast<uint8_t>(msgType);
					memset(m_Buffer + CKSUM_INDEX, 0, CKSUM_SIZE);
					memcpy(m_Buffer + DATA_INDEX, data, amt);
					uint16_t cksum = Math::Crc16::XModemCrc(m_Buffer, 0, msgSize);
					cksum = ArduinoExt::htons(cksum);
					memcpy(m_Buffer + CKSUM_INDEX, &cksum, CKSUM_SIZE);
					size_t retVal = Serial.write(m_Buffer, msgSize);
					Serial.flush();
					return retVal;
				}
			};
		}
	}
}

#endif //ARDUINOEXT_COMMS_USB_ARDUINOUSB_H__6_16_2021
