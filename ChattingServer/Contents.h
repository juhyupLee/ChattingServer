#pragma once
#include "SelectServer.h"
#include "SerializeBuffer.h"
#include "RingBuffer.h"
#include <unordered_map>

#define MAX_USER 5
#define MAX_ROOM 100
#define MAX_ROOM_USER_CNT 10
struct User
{
	Session* _Session;
	WCHAR _NickName[dfNICK_MAX_LEN];
	DWORD _ID;
	bool _Lobby;
	DWORD _RoomNum;
};

struct Room
{
	DWORD _NO;
	WCHAR _RoomName[ROOM_MAX_LEN];
	std::list<User*> _UserList;
};
extern std::unordered_map<DWORD, User*> g_UserMap;
extern std::unordered_map<DWORD, Room*> g_RoomMap;
extern DWORD g_RoomID;

BYTE AddUser(Session* curSession,WCHAR* nickName);
Room* CreateRoom(DWORD userID, WCHAR* roomName, BYTE* result);
BYTE RoomEnter(DWORD roomNo, DWORD userID);


Room* FindRoom(DWORD roomNo);
User* FindUser(DWORD userID);
bool LeaveRoom(DWORD userID);
void RemoveRoom(DWORD roomNo, DWORD userID);
