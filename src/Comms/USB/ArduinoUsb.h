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

#include "MessageBase.h"
#include "MessageString.h"
#include "MessageAck.h"

namespace ArduinoExt
{
	namespace Comms
	{
		namespace USB
		{
			class ArduinoUsb
			{
			public:
				static const uint8_t HEADER_SIZE = 6;
				static const uint8_t MAX_BUFFER_SIZE = 64;
				static const uint8_t NEED_ACK = 0x80; // 1000 0000
				static const uint8_t ACK_UPON_RECEIVE = 0x40; // 0100 0000
				static const uint8_t MSG_TYPE_MASK = 0x7F; // 0111 1111
			private:
				uint8_t m_Buffer[MAX_BUFFER_SIZE];
				uint16_t m_MessageId;
				uint8_t m_ReceivedArray[MAX_BUFFER_SIZE*4];
				ArduinoExt::Data::Buffer<uint8_t> m_ReceivedBuffer;
			public:
				/// \brief Constructor
				ArduinoUsb():
					m_Buffer(),
					m_MessageId(0),
					m_ReceivedArray(),
					m_ReceivedBuffer(m_ReceivedArray, MAX_BUFFER_SIZE*4)
				{}

				/// \brief Get the next message in the message queue.
				/// 
				/// Get the next message in the message queue. This function returns a pointer to a 
				/// MessageBase class. This pointer should NEVER BE DELETED. This class will manage the 
				/// memory of the pointer.
				/// \return A pointer to the parsed message.
				MessageBase* GetNextMessage()
				{
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
							return NULL;
						}
						else
						{
							MessageBase* retVal = NULL;
							// parse out message
							uint8_t* rawData = m_ReceivedBuffer.GetData();
							uint16_t cksum;
							memcpy(&cksum, rawData + 4, 2);
							cksum = ArduinoExt::ntohs(cksum);
							rawData[4] = 0;
							rawData[5] = 0;

							uint16_t check_cksum = Math::Crc16::XModemCrc(rawData, 0, msgSize);
							if (check_cksum != cksum)
							{
								SendString(F("Cksum mismatch"));
							}
							else
							{
								uint8_t msgType = ManagementByteMsgType(rawData[1]);
								retVal = GetMessageInstance(msgType);

								// parse
								if (retVal != NULL)
								{
									size_t parseResult = retVal->Parse(rawData, msgSize, HEADER_SIZE);
									if (parseResult != 0)
									{
										String errString = F("Error parsing message.\nType: ");
										errString += msgType;
										errString += F("\nRet: ");
										errString += parseResult;
										SendString(errString.c_str());
									}
								}
								else
								{
									String errString = F("Message Type '");
									errString += ManagementByteMsgType(rawData[1]);
									errString += F("' not supported.");
									SendString(errString.c_str());
								}
								
								bool needAck = ManagementByteNeedAck(rawData[1]);
								bool ackUponReceive = ManagementByteAckUponReceive(rawData[1]);
								if (needAck)
								{
									uint16_t nmid;
									memcpy(&nmid, m_Buffer + 2, 2);
									nmid = ArduinoExt::ntohs(nmid);
									if (retVal != NULL)
									{
										if (ackUponReceive)
										{
											SendAckSuccess(msgType, nmid);
										}
										else
										{
											retVal->MessageId = nmid;
											retVal->AckUponReceive = ackUponReceive;
										}
									}
									else
									{
										SendAckFail(msgType, nmid, 1);
									}
								}
							}
							m_ReceivedBuffer.Remove(msgSize);
							return retVal;
						}
					}

					return NULL;
				}

				virtual MessageBase* GetMessageInstance(uint8_t msgType)
				{
					return NULL;
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

				/// \brief Send a message accross the usb connection
				/// This will add the appropriate header and serialize the data before 
				/// sending it on the wire. A checksum value will be added to ensure the data gets
				/// to the destination intact.
				/// \param msg A pointer to a message object.
				/// \return The amount of data sent on the wire.
				size_t Send(MessageBase* msg)
				{
					if (msg == NULL) return -1;
					uint8_t msgSize = msg->MessageSize() + HEADER_SIZE;
					if (Serial.availableForWrite() < msgSize) return -2;
					m_Buffer[0] = msgSize;
					m_Buffer[1] = BuildManagementByte(msg->MsgTypeId(), msg->RequireAck(), msg->AckUponReceive);
					uint16_t netMsgId = m_MessageId;
					++m_MessageId;
					netMsgId = ArduinoExt::htons(netMsgId);
					memcpy(m_Buffer+2, &netMsgId, 2);
					
					m_Buffer[4] = 0;
					m_Buffer[5] = 0;
					size_t retVal = msg->ToBuffer(m_Buffer, MAX_BUFFER_SIZE, 6);
					if (retVal <= 0) return retVal;
					uint16_t cksum = Math::Crc16::XModemCrc(m_Buffer, 0, msgSize);
					cksum = ArduinoExt::htons(cksum);
					memcpy(m_Buffer + 4, &cksum, 2);
					retVal = Serial.write(m_Buffer, msgSize);
					Serial.flush();
					return retVal;
				}

				/// \brief Send an ack message indicating success
				/// \param msg The message we want to ack as success.
				inline void SendAckSuccess(MessageBase* msg)
				{
					SendAckSuccess(msg->MsgTypeId(), msg->MessageId);
				}

				/// \brief Send an ack message indicating success
				/// \param msgType The message time to ack
				/// \param id The id number of the message to ack.
				void SendAckSuccess(uint8_t msgType, uint16_t id)
				{
					MessageAckSuccess success;
					success.TypeToAck = msgType;
					success.MessageAckId = id;
					success.Error = 0;
					Send(&success);
				}

				/// \brief Send an ack message indicating failure.
				/// \param msg The message that failed.
				/// \param err The error code for the message.
				inline void SendAckFail(MessageBase* msg, uint8_t err)
				{
					SendAckFail(msg->MsgTypeId(), msg->MessageId, err);
				}

				/// \brief Send an ack message indicating failure.
				/// \param msgType The message time to ack
				/// \param id The id number of the message to ack.
				/// \param err The error code for the message.
				void SendAckFail(uint8_t msgType, uint16_t id, uint8_t err)
				{
					MessageAckFail fail;
					fail.TypeToAck = msgType;
					fail.MessageAckId = id;
					fail.Error = err;
					Send(&fail);
				}

				/// \brief Send a string accross the usb connection
				/// Send a string on the usb connection. This should be interpreted as 
				/// debug statements and should just be displayed by the receiver.
				/// \param msg A string to send.
				/// \return The amount of data sent on the wire.
				size_t SendString(const char * msg)
				{
					MessageString msgstr(msg);
					return Send(&msgstr);
				}

				/// \brief Send a string accross the usb connection
				/// Send a string on the usb connection. This should be interpreted as 
				/// debug statements and should just be displayed by the receiver.
				/// \param msg A string to send.
				/// \return The amount of data sent on the wire.
				size_t SendString(const __FlashStringHelper* msg)
				{
					MessageStringF msgStr(msg);
					return Send(&msgStr);
				}

			private:
				/// \brief the managment byte contains all the important information on when and 
				/// how to ack the packet.
				uint8_t BuildManagementByte(uint8_t msgType, bool needAck, bool ackUponReceive)
				{
					uint8_t retVal = 0;
					
					if (needAck)
					{
						retVal = NEED_ACK;
						if (ackUponReceive)
						{
							retVal |= ACK_UPON_RECEIVE;
						}
					}

					uint8_t msgTypeCheck = msgType;
					if (msgTypeCheck > static_cast<uint8_t>(MessageType::MESSAGE_TYPE_MAX))
					{
						msgTypeCheck = static_cast<uint8_t>(MessageType::MESSAGE_TYPE_MAX);
						SendString(F("Message type > MESSAGE_TYPE_MAX."));
					}

					retVal |= msgTypeCheck;
					return retVal;
				}

				static bool ManagementByteNeedAck(uint8_t byteData)
				{
					bool retVal = (byteData & NEED_ACK) != 0;
					return retVal;
				}

				static uint8_t ManagementByteMsgType(uint8_t byteData)
				{
					return byteData & MSG_TYPE_MASK;
				}

				static bool ManagementByteAckUponReceive(uint8_t byteData)
				{
					bool retVal = (byteData & ACK_UPON_RECEIVE) != 0;
					return retVal;
				}
			};
		}
	}
}

#endif //ARDUINOEXT_COMMS_USB_ARDUINOUSB_H__6_16_2021
