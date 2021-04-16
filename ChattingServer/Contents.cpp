#include "Contents.h"
#include "Log.h"
#include "PacketProcess.h"
std::unordered_map<DWORD, User*> g_UserMap;
std::unordered_map<DWORD, Room*> g_RoomMap;
DWORD g_RoomID = 0;

BYTE AddUser(Session* curSession, WCHAR* nickName)
{
	if (g_UserMap.size() >= MAX_USER)
	{
		//-----------------------------------
		// 사용자 초과 : 3
		//-----------------------------------
		return  3;
	}
	auto iter = g_UserMap.find(curSession->_ID);
	User* user = FindUser(curSession->_ID);

	//--------------------------------------------
	// 신규유저추가를 해야되는데, UserMap에 이미 해당 세션이있다면
	// AddUser는 Fail이다.
	//--------------------------------------------
	if (user != nullptr)
	{
		//-----------------------------------
		// 기타오류 : 4
		//-----------------------------------

		return 4;
	}

	//--------------------------------------------------------
	// 중복 닉네임 체크
	//--------------------------------------------------------
	iter = g_UserMap.begin();

	for (; iter != g_UserMap.end();)
	{
		User* user= (*iter).second;
		if (!wcscmp(nickName, user->_NickName))
		{
			//-----------------------------------
			// 중복 닉네임 :2
			//-----------------------------------
			return 2;
		}
		++iter;
	}

	User* newUser = new User;
	newUser->_ID = curSession->_ID;
	newUser->_Lobby = true;
	newUser->_Session = curSession;
	wcscpy_s(newUser->_NickName, nickName);

	g_UserMap.insert(std::make_pair(curSession->_ID, newUser));
	_LOG(LOG_LEVEL_DEBUG, L"User:%s 추가", nickName);
	//-----------------------------------
	// 유저 추가 OK :1
	//-----------------------------------
	return 1;


}

Room* CreateRoom(DWORD userID,WCHAR* roomName,BYTE* result)
{
	auto userIter = g_UserMap.find(userID);
	
	//-----------------------------------
	// 유저가 로그인이 안되어있거나, 로비에 없다면 오류임.
	//-----------------------------------
	if (userIter == g_UserMap.end())
	{
		*result = 4;
		return nullptr;
	}
	
	User* curUser = (*userIter).second;
	if (!curUser->_Lobby)
	{
		*result = 4;
		return nullptr;
	}
	if (MAX_ROOM < g_RoomMap.size())
	{
		//----------------------------------------------
		// 갯수 초과 :3
		//----------------------------------------------
		*result = 3;
		return nullptr;

	}
	if (wcslen(roomName) > ROOM_MAX_LEN - 1)
	{
		//----------------------------------------------
		// 기타(방길이 초과) :4
		//----------------------------------------------
		*result = 4;
		return nullptr;
	}

	auto iter = g_RoomMap.begin();

	for (; iter != g_RoomMap.end();)
	{
		Room* curRoom = (*iter).second;

		if (!wcscmp(curRoom->_RoomName, roomName))
		{
			//----------------------------------------------
			// 방이름 중복 :2
			//----------------------------------------------
			*result = 2;
			return nullptr;
		}
		++iter;
	}

	Room* newRoom = new Room;
	newRoom->_NO = g_RoomID++;
	g_RoomMap.insert(std::make_pair(newRoom->_NO, newRoom));
	wcscpy_s(newRoom->_RoomName, roomName);


	//----------------------------------------------
	// Okay :1
	//----------------------------------------------
	*result = 1;
	return newRoom;

}

BYTE RoomEnter(DWORD roomNo, DWORD userID)
{
	
	Room* curRoom = FindRoom(roomNo);

	if (curRoom == nullptr)
	{
		return df_RESULT_ROOM_ENTER_ETC;
	}
	
	User* curUser = FindUser(userID);

	if (curUser == nullptr)
	{
		return df_RESULT_ROOM_ENTER_ETC;
	}

	if (!curUser->_Lobby)
	{
		return df_RESULT_ROOM_ENTER_ETC;
	}

	if (curRoom->_UserList.size() >= MAX_ROOM_USER_CNT)
	{
		return df_RESULT_ROOM_ENTER_MAX;
	}

	curUser->_Lobby = false;
	curUser->_RoomNum = roomNo;
	curRoom->_UserList.push_back(curUser);
	
	return df_RESULT_ROOM_ENTER_OK;
}

Room* FindRoom(DWORD roomNo)
{
	auto iter = g_RoomMap.find(roomNo);
	if (iter == g_RoomMap.end())
	{
		return nullptr;
	}

	return (*iter).second;
}

User* FindUser(DWORD userID)
{
	auto iter = g_UserMap.find(userID);
	if (iter == g_UserMap.end())
	{
		return nullptr;
	}

	return (*iter).second;
}

bool LeaveRoom(DWORD userID)
{
	User* curUser = FindUser(userID);
	if (curUser == nullptr)
	{
		return false;
	}
	if (curUser->_Lobby)
	{
		return false;
	}

	Room* curRoom = FindRoom(curUser->_RoomNum);
	if (curRoom == nullptr)
	{
		return false;
	}

	auto userIter = curRoom->_UserList.begin();
	bool bLeave = false;

	for (; userIter != curRoom->_UserList.end(); ++userIter)
	{
		User* curUser = *userIter;
		if (curUser->_ID == userID)
		{
			bLeave = true;
			curUser->_Lobby = true;
			curRoom->_UserList.erase(userIter);

			if (curRoom->_UserList.size() == 0)
			{
				RemoveRoom(curUser->_RoomNum,userID);
			}
			break;
		}
	}
	if (!bLeave)
	{
		return false;
	}
	return true;
}

void RemoveRoom(DWORD roomNo,DWORD userID)
{
	Room* delRoom = FindRoom(roomNo);
	if (delRoom == nullptr)
	{
		_LOG(LOG_LEVEL_DEBUG, L"FindRoom Fail");
		return;
	}

	auto iter = g_RoomMap.begin();

	for (; iter != g_RoomMap.end();)
	{
		Room* curRoom = (*iter).second;

		if (delRoom == curRoom)
		{
			delete curRoom;
			g_RoomMap.erase(iter);
			break;
		}
	}
	SerializeBuffer packetToSend;
	st_PACKET_HEADER header;
	MakePacket_RES_DELETE(&header,&packetToSend, roomNo);

	User* curUser = FindUser(userID);
	if (curUser == nullptr)
	{
		_LOG(LOG_LEVEL_DEBUG, L"FindUser Fail");
		return;
	}
	Session* curSession = curUser->_Session;

	SendToBroadcast(&header ,&packetToSend, nullptr);

}
