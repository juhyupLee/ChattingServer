#pragma once
#include <iostream>
class RingQ
{

public:
	enum
	{
		RING_BUFFER_SIZE = 1000
	};

public:
	RingQ();
	~RingQ();
public:
	int Enqueue(char* buffer, int32_t size);
	int Dequeue(char* buffer, int32_t size);
	//bool Enqueue(char* buffer, int32_t size);
	//bool Dequeue(char* buffer, int32_t size);
	

	int32_t GetFreeSize() const; 
	int32_t GetUsedSize() const;


	int32_t GetDirectEnqueueSize()const;
	int32_t GetDirectDequeueSize()const;
	void ClearBuffer();

	void MoveRear(int size);
	void MoveFront(int size);
	char* GetFrontBufferPtr(void);
	char* GetRearBufferPtr(void);
	int Peek(char* dest, int size);

	int32_t GetFront();
	int32_t GetRear();

	int32_t GetDirectEnqueueSize(int32_t copyFront, int32_t copyRear)const;
	int32_t GetDirectDequeueSize(int32_t copyFront, int32_t copyRear)const;

private:
	int32_t GetFreeSize(int32_t front, int32_t rear) const;
	int32_t GetUsedSize(int32_t front, int32_t rear) const;

private:
	int32_t m_Front;
	int32_t m_Rear;
	char m_Buffer[RING_BUFFER_SIZE];

};