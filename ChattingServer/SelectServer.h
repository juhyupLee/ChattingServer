#pragma once
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include "SerializeBuffer.h"
#include "RingBuffer.h"
#include <unordered_map>
#include "Protocol.h"


extern SOCKET g_ListenSocket;

struct Session
{
	SOCKET _Socket;
	DWORD _ID;
	RingQ _RecvRingQ;
	RingQ _SendRingQ;
};

extern std::unordered_map<SOCKET, Session*> g_SessionMap;
extern DWORD g_SessionID;

void Init_Network();
void NetworkProcess();
void SelectProcess(SOCKET* socketTable, fd_set* readSet, fd_set* writeSet);
void RecvProcess(SOCKET socket);
bool SendProcess(SOCKET socket);
void AcceptUser();

Session* FindSession(SOCKET socket);
Session* FindSession(DWORD id);
void Disconnect(SOCKET socket);

//-----------------------------
// 방에있는 모든 인원에게 패킷전달
//-----------------------------
void SendToRoom(DWORD roomNo, st_PACKET_HEADER* header,SerializeBuffer* packet, Session* exceptSession=nullptr);
void SendToUnicast(Session* curSession, st_PACKET_HEADER* header, SerializeBuffer* packet);
void SendToBroadcast(st_PACKET_HEADER* header,SerializeBuffer* packet, Session* exceptSession = nullptr);
DWORD FindRoomNumber(Session* session);

