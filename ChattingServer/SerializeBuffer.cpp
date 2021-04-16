#include "SerializeBuffer.h"

SerializeBuffer::SerializeBuffer()
    :
    m_FreeSize(BUFFER_SIZE),
    m_UsedSize(0),
    m_WritePos(0),
    m_ReadPos(0),
    m_Buffer{0,}
{
}

SerializeBuffer::~SerializeBuffer()
{
}

void SerializeBuffer::Release()
{
}

void SerializeBuffer::Clear()
{
    m_FreeSize = BUFFER_SIZE;
    m_UsedSize = 0;
    m_ReadPos = 0;
    m_WritePos = 0;
}

int32_t SerializeBuffer::GetBufferSize()
{
    return m_FreeSize;
}

int32_t SerializeBuffer::GetDataSize()
{
    return m_UsedSize;
}

char* SerializeBuffer::GetBufferPtr(void)
{
    return m_Buffer;
}

int32_t SerializeBuffer::MoveWritePos(int size)
{
    if (size < 0)
    {
        return 0;
    }
    if (m_FreeSize <= size)
    {
        size = m_FreeSize;
    }

    m_WritePos += size;
    m_UsedSize += size;
    m_FreeSize -= size;

    return size;
}

int32_t SerializeBuffer::MoveReadPos(int size)
{
    if (size < 0)
    {
        return 0;
    }
    if (m_UsedSize <= size)
    {
        size = m_UsedSize;
    }
    m_ReadPos += size;
    m_UsedSize -= size;
    m_FreeSize += size;
    return size;
}

//---------------------------------------------------------------------
// Data -> SerializeBuffer
//---------------------------------------------------------------------

SerializeBuffer& SerializeBuffer::operator<<(BYTE inValue)
{
    if (m_FreeSize < sizeof(BYTE))
    {
        throw SerializeBufferExcept(L"BYTE: 여유공간이 없습니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(m_Buffer+m_WritePos, &inValue, sizeof(BYTE));

    m_WritePos += sizeof(BYTE);
    m_UsedSize += sizeof(BYTE);
    m_FreeSize -= sizeof(BYTE);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(char inValue)
{
    if (m_FreeSize < sizeof(char))
    {
        throw SerializeBufferExcept(L"char: 여유공간이 없습니다.", L"SerilizeBuffer.cpp", __LINE__);
    }
    memcpy(m_Buffer + m_WritePos, &inValue, sizeof(char));

    m_WritePos += sizeof(char);
    m_UsedSize += sizeof(char);
    m_FreeSize -= sizeof(char);

    return *this;

}

SerializeBuffer& SerializeBuffer::operator<<(short inValue)
{
    if (m_FreeSize < sizeof(short))
    {
        throw SerializeBufferExcept(L"short: 여유공간이 없습니다.", L"SerilizeBuffer.cpp", __LINE__);
    }
    memcpy(m_Buffer + m_WritePos, &inValue, sizeof(short));

    m_WritePos += sizeof(short);
    m_UsedSize += sizeof(short);
    m_FreeSize -= sizeof(short);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(WORD inValue)
{
    if (m_FreeSize < sizeof(WORD))
    {
        throw SerializeBufferExcept(L"WORD: 여유공간이 없습니다.", L"SerilizeBuffer.cpp", __LINE__);
    }
    memcpy(m_Buffer + m_WritePos, &inValue, sizeof(WORD));

    m_WritePos += sizeof(WORD);
    m_UsedSize += sizeof(WORD);
    m_FreeSize -= sizeof(WORD);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(int32_t inValue)
{
    if (m_FreeSize <sizeof(int32_t))
    {
        throw SerializeBufferExcept(L"int32: 여유공간이 없습니다.",L"SerilizeBuffer.cpp",__LINE__);
    }

    memcpy(m_Buffer + m_WritePos, &inValue, sizeof(int32_t));

    m_WritePos += sizeof(int32_t);
    m_UsedSize += sizeof(int32_t);
    m_FreeSize -= sizeof(int32_t);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(DWORD inValue)
{
    if (m_FreeSize < sizeof(DWORD))
    {
        throw SerializeBufferExcept(L"DWORD: 여유공간이 없습니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(m_Buffer + m_WritePos, &inValue, sizeof(DWORD));

    m_WritePos += sizeof(DWORD);
    m_UsedSize += sizeof(DWORD);
    m_FreeSize -= sizeof(DWORD);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(float inValue)
{
    if (m_FreeSize < sizeof(float))
    {
        throw SerializeBufferExcept(L"float: 여유공간이 없습니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(m_Buffer + m_WritePos, &inValue, sizeof(float));

    m_WritePos += sizeof(float);
    m_UsedSize += sizeof(float);
    m_FreeSize -= sizeof(float);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(int64_t inValue)
{
    if (m_FreeSize < sizeof(int64_t))
    {
        throw SerializeBufferExcept(L"int64_t: 여유공간이 없습니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(m_Buffer + m_WritePos, &inValue, sizeof(int64_t));

    m_WritePos += sizeof(int64_t);
    m_UsedSize += sizeof(int64_t);
    m_FreeSize -= sizeof(int64_t);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator<<(double inValue)
{
    if (m_FreeSize < sizeof(double))
    {
        throw SerializeBufferExcept(L"double: 여유공간이 없습니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(m_Buffer + m_WritePos, &inValue, sizeof(double));

    m_WritePos += sizeof(double);
    m_UsedSize += sizeof(double);
    m_FreeSize -= sizeof(double);

    return *this;
}

//---------------------------------------------------------------------
// SerializeBuffer-->Data 
//---------------------------------------------------------------------
SerializeBuffer& SerializeBuffer::operator>>(BYTE& outValue)
{
    //----------------------------------
   // 버퍼에있는 데이터가 0보다 커야 버퍼에서뽑을 수 있다.
   // 그렇지 않다면 로그를 남기거나 종료를 시킨다.
   //----------------------------------
    if (m_UsedSize < sizeof(BYTE))
    {
        throw SerializeBufferExcept(L"BYTE: 사용중인 공간이 부족합니다.", L"SerilizeBuffer.cpp", __LINE__);
    }
    memcpy(&outValue, m_Buffer + m_ReadPos, sizeof(BYTE));

    m_ReadPos += sizeof(BYTE);
    m_UsedSize -= sizeof(BYTE);
    m_FreeSize += sizeof(BYTE);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(char& outValue)
{
    //----------------------------------
  // 버퍼에있는 데이터가 0보다 커야 버퍼에서뽑을 수 있다.
  // 그렇지 않다면 로그를 남기거나 종료를 시킨다.
  //----------------------------------
    if (m_UsedSize < sizeof(char))
    {
        throw SerializeBufferExcept(L"char: 사용중인 공간이 부족합니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(&outValue, m_Buffer + m_ReadPos, sizeof(char));

    m_ReadPos += sizeof(char);
    m_UsedSize -= sizeof(char);
    m_FreeSize += sizeof(char);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(short& outValue)
{
    //----------------------------------
  // 버퍼에있는 데이터가 0보다 커야 버퍼에서뽑을 수 있다.
  // 그렇지 않다면 로그를 남기거나 종료를 시킨다.
  //----------------------------------
    if (m_UsedSize < sizeof(short))
    {
        throw SerializeBufferExcept(L"short: 사용중인 공간이 부족합니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(&outValue, m_Buffer + m_ReadPos, sizeof(short));

    m_ReadPos += sizeof(short);
    m_UsedSize -= sizeof(short);
    m_FreeSize += sizeof(short);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(WORD& outValue)
{
    //----------------------------------
 // 버퍼에있는 데이터가 0보다 커야 버퍼에서뽑을 수 있다.
 // 그렇지 않다면 로그를 남기거나 종료를 시킨다.
 //----------------------------------
    if (m_UsedSize < sizeof(WORD))
    {
        throw SerializeBufferExcept(L"WORD: 사용중인 공간이 부족합니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(&outValue, m_Buffer + m_ReadPos, sizeof(WORD));

    m_ReadPos += sizeof(WORD);
    m_UsedSize -= sizeof(WORD);
    m_FreeSize += sizeof(WORD);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(int32_t& outValue)
{
    
    //----------------------------------
    // 버퍼에있는 데이터가 0보다 커야 버퍼에서뽑을 수 있다.
    // 그렇지 않다면 로그를 남기거나 종료를 시킨다.
    //----------------------------------
    if (m_UsedSize < sizeof(int32_t))
    {
        throw SerializeBufferExcept(L"int32_t: 사용중인 공간이 부족합니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(&outValue,m_Buffer + m_ReadPos,sizeof(int32_t));

    m_ReadPos += sizeof(int32_t);
    m_UsedSize -= sizeof(int32_t);
    m_FreeSize += sizeof(int32_t);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(DWORD& outValue)
{

    //----------------------------------
    // 버퍼에있는 데이터가 0보다 커야 버퍼에서뽑을 수 있다.
    // 그렇지 않다면 로그를 남기거나 종료를 시킨다.
    //----------------------------------
    if (m_UsedSize < sizeof(DWORD))
    {
        throw SerializeBufferExcept(L"DWORD: 사용중인 공간이 부족합니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(&outValue, m_Buffer + m_ReadPos, sizeof(DWORD));

    m_ReadPos += sizeof(DWORD);
    m_UsedSize -= sizeof(DWORD);
    m_FreeSize += sizeof(DWORD);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(float& outValue)
{
    //----------------------------------
  // 버퍼에있는 데이터가 0보다 커야 버퍼에서뽑을 수 있다.
  // 그렇지 않다면 로그를 남기거나 종료를 시킨다.
  //----------------------------------
    if (m_UsedSize < sizeof(float))
    {
        throw SerializeBufferExcept(L"float: 사용중인 공간이 부족합니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(&outValue, m_Buffer + m_ReadPos, sizeof(float));

    m_ReadPos += sizeof(float);
    m_UsedSize -= sizeof(float);
    m_FreeSize += sizeof(float);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(int64_t& outValue)
{
    //----------------------------------
// 버퍼에있는 데이터가 0보다 커야 버퍼에서뽑을 수 있다.
// 그렇지 않다면 로그를 남기거나 종료를 시킨다.
//----------------------------------
    if (m_UsedSize < sizeof(int64_t))
    {
        throw SerializeBufferExcept(L"int64_t: 사용중인 공간이 부족합니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(&outValue, m_Buffer + m_ReadPos, sizeof(int64_t));

    m_ReadPos += sizeof(int64_t);
    m_UsedSize -= sizeof(int64_t);
    m_FreeSize += sizeof(int64_t);

    return *this;
}

SerializeBuffer& SerializeBuffer::operator>>(double& outValue)
{
    //----------------------------------
// 버퍼에있는 데이터가 0보다 커야 버퍼에서뽑을 수 있다.
// 그렇지 않다면 로그를 남기거나 종료를 시킨다.
//----------------------------------
    if (m_UsedSize < sizeof(double))
    {
        throw SerializeBufferExcept(L"double: 사용중인 공간이 부족합니다.", L"SerilizeBuffer.cpp", __LINE__);
    }

    memcpy(&outValue, m_Buffer + m_ReadPos, sizeof(double));

    m_ReadPos += sizeof(double);
    m_UsedSize -= sizeof(double);
    m_FreeSize += sizeof(double);

    return *this;
}

int32_t SerializeBuffer::GetData(char* dest, int size)
{
    if (m_UsedSize <= size)
    {
        size = m_UsedSize;
    }

    memcpy(dest, m_Buffer + m_ReadPos, size);
    m_ReadPos += size;
    m_UsedSize -= size;
    m_FreeSize += size;

    return size;
}

int32_t SerializeBuffer::PutData(char* src, int size)
{
    if (m_FreeSize < size)
    {
        size = m_FreeSize;
    }

    memcpy(m_Buffer + m_WritePos,src, size);

    m_WritePos += size;
    m_UsedSize += size;
    m_FreeSize -= size;
    return size;
}