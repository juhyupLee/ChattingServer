#include "PacketProcess.h"
#include "Contents.h"
#include "Log.h"

void ProcessPacket_REQ_LOGIN(SerializeBuffer* packet,Session* curSession)
{
	//------------------------------------------------------
	// 클라로부터 로그인 패킷이오면 유저를 추가 한뒤, 유저 추가에대한 결과 패킷을
	// 클라로 보낸다.
	//------------------------------------------------------
	WCHAR nickName[dfNICK_MAX_LEN];
	(*packet).GetData((char*)nickName, dfNICK_MAX_LEN * 2);

	BYTE addUserResult =AddUser(curSession, nickName);

	st_PACKET_HEADER header;
	SerializeBuffer packetToSend;

	MakePacket_RES_LOGIN(&header, &packetToSend,addUserResult, curSession->_ID);
	SendToUnicast(curSession, &header, &packetToSend);
}

void ProcessPacket_REQ_ROOMLIST(Session* curSession)
{
	st_PACKET_HEADER header;
	SerializeBuffer packetToSend;

	MakePacket_RES_ROOMLIST(&header,&packetToSend);
	SendToUnicast(curSession, &header, &packetToSend);
}

void ProcessPacket_REQ_ROOMCREATE(SerializeBuffer* packet, Session* curSession)
{
	int16_t roomNameSize = 0;
	WCHAR roomName[ROOM_MAX_LEN] = { 0, };
	

	*packet >> roomNameSize;
	(*packet).GetData((char*)roomName, roomNameSize);


	BYTE createRoomResult = 0;

	Room* newRoom = CreateRoom(curSession->_ID,roomName, &createRoomResult);

	st_PACKET_HEADER header;
	SerializeBuffer packetToSend;

	if (newRoom == nullptr)
	{
		MakePacket_RES_ROOMCREATE(&header,&packetToSend, createRoomResult, 0, roomNameSize, roomName);
	}
	else
	{
		MakePacket_RES_ROOMCREATE(&header,&packetToSend, createRoomResult, newRoom->_NO, wcslen(newRoom->_RoomName) * 2, newRoom->_RoomName);
	}
	
	SendToBroadcast(&header,&packetToSend);

}

void ProcessPacket_REQ_ROOMENTER(SerializeBuffer* packet, Session* curSession)
{
	DWORD roomNo = 0;
	*packet >> roomNo;
	BYTE result = RoomEnter(roomNo, curSession->_ID);

	SerializeBuffer packetToSend;
	st_PACKET_HEADER header;

	MakePacket_RES_ROOMENTER(&header,&packetToSend, result, roomNo);
	SendToUnicast(curSession, &header, &packetToSend);

	if (result == 1)
	{
		packetToSend.Clear();
		st_PACKET_HEADER header2;
		User* curUser = FindUser(curSession->_ID);
		if (curUser == nullptr)
		{
			_LOG(LOG_LEVEL_ERROR, L"Find User Fail");
		}

		MakePacket_RES_USERENTER(&header2,&packetToSend, curUser->_NickName, curUser->_ID);
		SendToRoom(curUser->_RoomNum, &header2, &packetToSend, curSession);
	}
	
}

void ProcessPacket_REQ_CHAT(SerializeBuffer* packet, Session* curSession)
{
	//일단 패킷을 받아서 메시지와 대화내용을 방에 뿌려야되는데 당사자 빼고 뿌려야된다.

	int16_t messageSize = 0;
	WCHAR chatMessage[1000] = { 0, };

	*packet >> messageSize;
	(*packet).GetData((char*)chatMessage, messageSize);

	st_PACKET_HEADER header;
	SerializeBuffer packetToSend;

	MakePacket_RES_CHAT(&header,&packetToSend, curSession->_ID, messageSize, chatMessage);

	User* curUser = FindUser(curSession->_ID);
	if (curUser == nullptr)
	{
		_LOG(LOG_LEVEL_ERROR, L"Find User Fail");
	}
	SendToRoom(curUser->_RoomNum, &header,&packetToSend, curSession);
}

void ProcessPacket_REQ_LEAVE(Session* curSession)
{
	// 방퇴장 들어온 세션이 어디방에있는지 찾아서 그방에서 유저정보를 빼낸다
	// 그리고 그 방에있는 세션들에게, 이 유저가나갔다는 패킷을 보내야함.

	st_PACKET_HEADER header;
	SerializeBuffer packetToSend;

	MakePacket_RES_LEAVE(&header,&packetToSend, curSession->_ID);

	User* curUser = FindUser(curSession->_ID);
	if (curUser == nullptr)
	{
		_LOG(LOG_LEVEL_ERROR, L"Find User Fail");
	}
	SendToRoom(curUser->_RoomNum, &header ,&packetToSend);

	if (!LeaveRoom(curSession->_ID))
	{
		_LOG(LOG_LEVEL_ERROR, L"방퇴장 Fail [session:%d]", curSession->_ID);
		return;
	}

}

void MakePacket_RES_LOGIN(st_PACKET_HEADER* header, SerializeBuffer* packet, BYTE loginResult, DWORD sessionID)
{
	(*header).byCode = dfPACKET_CODE;
	(*header).wMsgType = df_RES_LOGIN;
	(*header).wPayloadSize = sizeof(BYTE) + sizeof(DWORD);

	//------------------------------------------------------
	// 체크섬 구하기
	//------------------------------------------------------

	(*packet) << loginResult << sessionID;
	(*header).byCheckSum = GetCheckSum(packet, df_RES_LOGIN);

	_LOG(LOG_LEVEL_DEBUG, L"Make Login Packet(S->C) LoginResult[%d] SessionID[%d]", loginResult, sessionID);
}

void MakePacket_RES_ROOMLIST(st_PACKET_HEADER* header, SerializeBuffer* packet)
{
	
	(*header).byCode = dfPACKET_CODE;
	(*header).wMsgType = df_RES_ROOM_LIST;
	int16_t roomCount = g_RoomMap.size();
	*packet << roomCount;

	auto iter = g_RoomMap.begin();

	for (; iter != g_RoomMap.end();)
	{
		DWORD roomNo = (*iter).first;
		Room* curRoom = (*iter).second;

		*packet << roomNo;
		int16_t roomNameSize = wcslen(curRoom->_RoomName)*2;
		*packet << roomNameSize;
		(*packet).PutData((char*)curRoom->_RoomName, roomNameSize);

		BYTE joinUserCount = curRoom->_UserList.size();
		
		*packet << joinUserCount;

		auto userListIter = curRoom->_UserList.begin();

		for (; userListIter != curRoom->_UserList.end();)
		{
			(*packet).PutData((char*)(*userListIter)->_NickName, dfNICK_MAX_LEN * 2);
			++userListIter;
		}
		++iter;
	}
	(*header).wPayloadSize = (*packet).GetDataSize();
	(*header).byCheckSum = GetCheckSum(packet, df_RES_ROOM_LIST);
}

void MakePacket_RES_ROOMCREATE(st_PACKET_HEADER* header,SerializeBuffer* packet, BYTE roomResult, DWORD roomNo, int16_t roomNameSize, WCHAR* roomName)
{
	(*header).byCode = dfPACKET_CODE;
	(*header).wMsgType = df_RES_ROOM_CREATE;
	(*header).wPayloadSize = sizeof(BYTE) + sizeof(DWORD) + sizeof(int16_t) + roomNameSize;

	(*packet) << roomResult << roomNo << roomNameSize;
	(*packet).PutData((char*)roomName, roomNameSize);

	(*header).byCheckSum = GetCheckSum(packet, df_RES_ROOM_CREATE);
}

void MakePacket_RES_ROOMENTER(st_PACKET_HEADER* header,SerializeBuffer* packet, BYTE result, DWORD roomNo)
{
	(*header).byCode = dfPACKET_CODE;
	(*header).wMsgType = df_RES_ROOM_ENTER;
	
	if (result != df_RESULT_ROOM_ENTER_OK)
	{
		(*header).wPayloadSize = sizeof(BYTE);
		(*packet).PutData((char*)&header, sizeof(st_PACKET_HEADER));
		(*packet) << result;

		(*header).byCheckSum = GetCheckSum(packet, df_RES_ROOM_ENTER);
		return;
	}

	Room* curRoom = FindRoom(roomNo);

	int16_t roomNameSize = wcslen(curRoom->_RoomName) * 2;
	(*packet) << result<< roomNo << roomNameSize;
	(*packet).PutData((char*)curRoom->_RoomName, roomNameSize);

	auto userIter = curRoom->_UserList.begin();

	BYTE joinUserCount = curRoom->_UserList.size();

	(*packet) << joinUserCount;

	for (; userIter != curRoom->_UserList.end();)
	{
		User* curUser = *userIter;
		(*packet).PutData((char*)curUser->_NickName, dfNICK_MAX_LEN*2);
		(*packet) << curUser->_ID;
		++userIter;
	}

	(*header).wPayloadSize = (*packet).GetDataSize();
	(*header).byCheckSum = GetCheckSum(packet, df_RES_ROOM_ENTER);
}

void MakePacket_RES_CHAT(st_PACKET_HEADER* header,SerializeBuffer* packet, DWORD userID, int16_t messageSize, WCHAR* message)
{
	(*header).byCode = dfPACKET_CODE;
	(*header).wMsgType = df_RES_CHAT;

	(*packet) << userID << messageSize;
	(*packet).PutData((char*)message, messageSize);

	(*header).wPayloadSize = (*packet).GetDataSize() ;
	(*header).byCheckSum = GetCheckSum(packet, df_RES_CHAT);
}

void MakePacket_RES_LEAVE(st_PACKET_HEADER* header,SerializeBuffer* packet, DWORD userID)
{
	(*header).byCode = dfPACKET_CODE;
	(*header).wMsgType = df_RES_ROOM_LEAVE;
	(*header).wPayloadSize = sizeof(DWORD);
	
	*packet << userID;

	(*header).byCheckSum = GetCheckSum(packet, df_RES_ROOM_LEAVE);

}


void MakePacket_RES_DELETE(st_PACKET_HEADER* header,SerializeBuffer* packet, DWORD roomNo)
{
	(*header).byCode = dfPACKET_CODE;
	(*header).wMsgType = df_RES_ROOM_DELETE;
	(*header).wPayloadSize = sizeof(DWORD);

	*packet << roomNo;

	(*header).byCheckSum = GetCheckSum(packet, df_RES_ROOM_DELETE);
}

void MakePacket_RES_USERENTER(st_PACKET_HEADER* header,SerializeBuffer* packet, WCHAR* nickName, DWORD userID)
{
	(*header).byCode = dfPACKET_CODE;
	(*header).wMsgType = df_RES_USER_ENTER;
	(*header).wPayloadSize = dfNICK_MAX_LEN *2+sizeof(DWORD);

	(*packet).PutData((char*)nickName, dfNICK_MAX_LEN * 2);
	(*packet) << userID;

	(*header).byCheckSum = GetCheckSum(packet, df_RES_USER_ENTER);
}

BYTE GetCheckSum(SerializeBuffer* packet,WORD messageType)
{
	BYTE checkSum = 0;
	BYTE* checkSumPtr = (BYTE*)&messageType;

	for (int i = 0; i < sizeof(WORD); ++i)
	{
		checkSum += *checkSumPtr;
		++checkSumPtr;
	}

	checkSumPtr = (BYTE*)((*packet).GetBufferPtr());

	for (int i = 0; i < (*packet).GetDataSize(); ++i)
	{
		checkSum += *checkSumPtr;
		++checkSumPtr;
	}
	return checkSum % 256;
}
