////////////////////////////////////////
// Written By: Spencer Hoffa
// File: Buffer.h
// Copyright: (c)2021
/// \copyright \link LICENSE.md MIT License\endlink 2021 Spencer Hoffa 
////////////////////////////////////////

#ifndef ARDUINOEXT_BUFFER_H__1_31_2021
#define ARDUINOEXT_BUFFER_H__1_31_2021

#ifndef ARDUINO
#include <cstring> // for memcpy
#endif

namespace ArduinoExt
{
	namespace Data
	{
		template <class T>
		class Buffer
		{
		private:
			T* m_Buffer;
			int m_BufferSize;
			int m_DataInBuffer;
		public:
			// this only manages a buffer... it doesn't create the data.
			// \param buffer the buffer
			// \param size the size of the buffer
			Buffer(T* buffer, int size) :
				m_Buffer(buffer),
				m_BufferSize(size),
				m_DataInBuffer(0)
			{ }

			~Buffer()
			{
				m_Buffer = NULL;
				m_BufferSize = 0;
				m_DataInBuffer = 0;
			}

			const T& operator[](size_t index) const 
			{
				return m_Buffer[index];
			}

			T& operator[](size_t index) 
			{
				return m_Buffer[index];
			}

			int AddData(T* dataArray, int dataInArray)
			{
				if (dataArray == NULL) return -1;
				if (dataInArray <= 0) return -2;
				if (m_DataInBuffer + dataInArray > m_BufferSize) return -3;

				memcpy(m_Buffer + m_DataInBuffer, dataArray, dataInArray);
				m_DataInBuffer += dataInArray;
				return dataInArray;
			}

			void Remove(int amount)
			{
				if (amount > 0)
				{
					memcpy(m_Buffer, m_Buffer + amount, m_DataInBuffer - amount);
					m_DataInBuffer -= amount;
				}
			}

			T* GetData()
			{
				return m_Buffer;
			}

			int GetDataAmount()
			{
				return m_DataInBuffer;
			}

			bool HasData()
			{
				return m_DataInBuffer > 0;
			}

			bool HasData(int amount)
			{
				if (amount <= 0) return false;

				return m_DataInBuffer >= amount;
			}

			T Peek(int index)
			{
				return m_Buffer[index];
			}
		};
	}
}

#endif //ARDUINOEXT_BUFFER_H__1_31_2021
