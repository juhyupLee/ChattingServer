#pragma once
#include "SelectServer.h"
#include "RingBuffer.h"
#include "SerializeBuffer.h"


void ProcessPacket_REQ_LOGIN(SerializeBuffer* packet, Session* curSession);
void ProcessPacket_REQ_ROOMLIST(Session* curSession);
void ProcessPacket_REQ_ROOMCREATE(SerializeBuffer* packet, Session* curSession);
void ProcessPacket_REQ_ROOMENTER(SerializeBuffer* packet, Session* curSession);
void ProcessPacket_REQ_CHAT(SerializeBuffer* packet, Session* curSession);
void ProcessPacket_REQ_LEAVE(Session* curSession);


void MakePacket_RES_LOGIN(st_PACKET_HEADER* header,SerializeBuffer* packet,BYTE loginResult, DWORD sessionID);
void MakePacket_RES_ROOMLIST(st_PACKET_HEADER* header, SerializeBuffer* packet);
void MakePacket_RES_ROOMCREATE(st_PACKET_HEADER* header,SerializeBuffer* packet, BYTE roomResult, DWORD roomNo,int16_t roomNameSize, WCHAR* roomName);
void MakePacket_RES_ROOMENTER(st_PACKET_HEADER* header,SerializeBuffer* packet, BYTE result, DWORD roomNo);
void MakePacket_RES_CHAT(st_PACKET_HEADER* header,SerializeBuffer* packet, DWORD userID, int16_t messageSize, WCHAR* message);
void MakePacket_RES_LEAVE(st_PACKET_HEADER* header,SerializeBuffer* packet, DWORD userID);

void MakePacket_RES_DELETE(st_PACKET_HEADER* header,SerializeBuffer* packet, DWORD roomNo);
void MakePacket_RES_USERENTER(st_PACKET_HEADER* header,SerializeBuffer* packet, WCHAR* nickName,DWORD userID);
BYTE GetCheckSum(SerializeBuffer* packet, WORD messageType);