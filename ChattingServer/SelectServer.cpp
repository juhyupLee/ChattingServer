#include "SelectServer.h"
#include "Log.h"
#include <locale>
#include "Contents.h"
#include "PacketProcess.h"

std::unordered_map<SOCKET, Session*> g_SessionMap;
SOCKET g_ListenSocket;
DWORD g_SessionID = 0;

void Init_Network()
{
	setlocale(LC_ALL, "");
	WSADATA wsaData;
	SOCKADDR_IN serverAddr;
	

	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		_LOG(LOG_LEVEL_ERROR, L"WSAStartUp() Error:%d",WSAGetLastError());
	}

	g_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (g_ListenSocket == INVALID_SOCKET)
	{
		_LOG(LOG_LEVEL_ERROR, L"socket() Error:%d", WSAGetLastError());
	}

	//-----------------------------------------------------
	// 내가알기로, select는 논블로킹으로 전환하는게 원칙으로알고있음.
	//-----------------------------------------------------
	u_long on = 1;
	ioctlsocket(g_ListenSocket, FIONBIO, &on);
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(dfNETWORK_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (0 != bind(g_ListenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		_LOG(LOG_LEVEL_ERROR, L"bind() Error:%d", WSAGetLastError());
	}

	if (0 != listen(g_ListenSocket, SOMAXCONN))
	{
		_LOG(LOG_LEVEL_ERROR, L"listen() Error:%d", WSAGetLastError());
	}

	_LOG(LOG_LEVEL_DEBUG, L"Server Listening.....");
}

void AcceptUser()
{
	SOCKADDR_IN clientAddr;
	memset(&clientAddr, 0, sizeof(SOCKADDR_IN));
	int clientAddrSize = sizeof(SOCKADDR_IN);

	SOCKET clientSocket = accept(g_ListenSocket, (sockaddr*)&clientAddr, &clientAddrSize);


	if (clientSocket == INVALID_SOCKET)
	{
		_LOG(LOG_LEVEL_ERROR, L"accept() error:%d", WSAGetLastError());
		return;
	}
	//----------------------------------------------------------
	// accept의 백로그큐에서 뽑았다면, 세션을 새로생성한다
	// 네트워크단에서 세션이만들어진거지, 아직 유저가 로그인한게 아니다.
	//----------------------------------------------------------
	Session* newSession = new Session;
	newSession->_ID = g_SessionID++;
	newSession->_Socket = clientSocket;

	//----------------------------------------------------------
	//linger Option 적용
	//----------------------------------------------------------

	linger lingerOpt;
	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;
	int setsockoptRtn = setsockopt(clientSocket, SOL_SOCKET, SO_LINGER, (const char*)&lingerOpt, sizeof(linger));
	if (setsockoptRtn != 0)
	{
		_LOG(LOG_LEVEL_ERROR, L"setsockopt() error:%d", WSAGetLastError());
	}

	//----------------------------------------------------------
	// Session Map의 추가.
	//----------------------------------------------------------
	g_SessionMap.insert(std::make_pair(clientSocket, newSession));

}

Session* FindSession(SOCKET socket)
{
	auto iter = g_SessionMap.find(socket);

	if (iter == g_SessionMap.end())
	{
		return nullptr;
	}

	return (*iter).second;
}

Session* FindSession(DWORD id)
{
	auto iter = g_SessionMap.begin();

	for (; iter != g_SessionMap.end();)
	{
		Session* curSession = (*iter).second;

		if (curSession->_ID == id)
		{
			return curSession;
		}
		++iter;
	}
	return nullptr;
}

void Disconnect(SOCKET socket)
{
	Session* delSession = FindSession(socket);
	if (delSession == nullptr)
	{
		_LOG(LOG_LEVEL_DEBUG, L"Find Session Fail");
		return; 
	}
	User* delUser = FindUser(delSession->_ID);

	//-------------------------------------------------
	// 세션도 있고, 유저도 현재있는 상황(정상상황) 
	// 세션만 있을 경우 세션정리, 소켓정리후 끝
	//-------------------------------------------------
	if (delUser != nullptr)
	{
		if (!delUser->_Lobby)
		{
			//--------------------------------------------------
			//유저가 방에있다면 Leave 처리를 한뒤, User맵에서 지운다.
			//--------------------------------------------------
			ProcessPacket_REQ_LEAVE(delSession);
		}
		//--------------------------------------------------
		//유저 맵에서 유저를 지운다.
		//--------------------------------------------------
		auto userMapiter = g_UserMap.begin();
		for (; userMapiter != g_UserMap.end(); ++userMapiter)
		{
			if ((*userMapiter).second == delUser)
			{
				delete delUser;
				g_UserMap.erase(userMapiter);
				break;
			}
		}
	}
	
	auto sessionIter = g_SessionMap.begin();
	for (; sessionIter != g_SessionMap.end(); ++sessionIter)
	{
		if ((*sessionIter).second == delSession)
		{
			delete delSession;
			g_SessionMap.erase(sessionIter);
			break;
		}
	}
	closesocket(socket);

}

void SendToRoom(DWORD roomNo, st_PACKET_HEADER* header, SerializeBuffer* packet, Session* exceptSession)
{
	Room* curRoom = FindRoom(roomNo);

	if (curRoom == nullptr)
	{
		_LOG(LOG_LEVEL_ERROR, L"Find Room Fail");
		return;
	}
	auto iter = curRoom->_UserList.begin();

	for (; iter != curRoom->_UserList.end();++iter)
	{
		User* curUser = *iter;
		Session* curSession = FindSession(curUser->_ID);

		if (exceptSession == curSession)
		{
			continue;
		}
		if (curSession == nullptr)
		{
			_LOG(LOG_LEVEL_ERROR, L"Find Session Fail");
			return;
		}
		SendToUnicast(curSession, header, packet);
	}
}

void SendToUnicast(Session* curSession, st_PACKET_HEADER* header,SerializeBuffer* packet)
{
	if (curSession->_SendRingQ.GetFreeSize() <= 0)
	{
		_LOG(LOG_LEVEL_ERROR, L"송신 링버퍼 FreeSize 0 [session:%d]", curSession->_ID);
	}

	int enQRtn = curSession->_SendRingQ.Enqueue((char*)header, sizeof(st_PACKET_HEADER));

	if (enQRtn < sizeof(st_PACKET_HEADER))
	{
		_LOG(LOG_LEVEL_ERROR, L"송신 링버퍼 부족 [session:%d]", curSession->_ID);
	}
	enQRtn = curSession->_SendRingQ.Enqueue((*packet).GetBufferPtr(), (*packet).GetDataSize());

	if (enQRtn < (*packet).GetDataSize())
	{
		_LOG(LOG_LEVEL_ERROR, L"송신 링버퍼 부족 [session:%d]", curSession->_ID);
	}
}

void SendToBroadcast(st_PACKET_HEADER* header,SerializeBuffer* packet, Session* exceptSession)
{
	auto iter = g_UserMap.begin();

	for (; iter != g_UserMap.end(); ++iter)
	{
		User* curUser = (*iter).second;
		Session* curSession = curUser->_Session;
		SendToUnicast(curSession, header, packet);
	}
}

DWORD FindRoomNumber(Session* session)
{
	auto iter = g_RoomMap.begin();

	for (; iter != g_RoomMap.end(); ++iter)
	{
		Room* curRoom = (*iter).second;
		auto userIter = curRoom->_UserList.begin();
		for (; userIter != curRoom->_UserList.end(); ++userIter)
		{
			User* curUser = *userIter;
			if (curUser->_ID == session->_ID)
			{
				return curRoom->_NO;
			}
		}

	}
	return -1;
}


void NetworkProcess()
{
	fd_set readSet;
	fd_set writeSet;
	SOCKET socketTable[FD_SETSIZE];
	int32_t sessionCount = 0;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);

	FD_SET(g_ListenSocket, &readSet);
	socketTable[sessionCount++] = g_ListenSocket;

	auto mapIter = g_SessionMap.begin();

	for (; mapIter != g_SessionMap.end();)
	{
		SOCKET sessionSocket = (*mapIter).first;
		++mapIter;
		//-------------------------------------------------------------
		// 여기서 ++iter을 미리해줘야지 만약에 마지막에 한다면, 중간에 SelectProcess동안 
		// 연결종료나 disconnect가 발생했을시, 문제가난다.(iter 오류가날듯)
		//-------------------------------------------------------------
		if (sessionCount >= FD_SETSIZE)
		{
			SelectProcess(socketTable, &readSet, &writeSet);
			sessionCount = 0;
			memset(socketTable, 0, sizeof(socketTable));
			FD_ZERO(&readSet);
			FD_ZERO(&writeSet);

			FD_SET(g_ListenSocket, &readSet);
			socketTable[sessionCount++] = g_ListenSocket;
		}
		socketTable[sessionCount++] = sessionSocket;
		FD_SET(sessionSocket, &readSet);
		FD_SET(sessionSocket, &writeSet);

	}
	if (sessionCount >= 1)
	{
		SelectProcess(socketTable, &readSet, &writeSet);
	}
	
}

void SelectProcess(SOCKET* socketTable, fd_set* readSet, fd_set* writeSet)
{
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_sec = 0;
	//----------------------------------------
	// 64개이상의 세션을 탐색해야될 경우, 만약 특정 64개부분에서 반응이없어 블락이걸린다면
	// 다른 세션에서 반응이 있는경우, 블락때문에 처리가안되는 문제가 생길 수 있다.
	//----------------------------------------
	int selectRtn = select(0, readSet, writeSet, nullptr, &timeVal);
	bool bSendCheck = false;

	if (selectRtn < 0)
	{
		_LOG(LOG_LEVEL_ERROR, L"select() error:%d", WSAGetLastError());
		return;
	}

	for (int i = 0; i < FD_SETSIZE && 0 < selectRtn; ++i)
	{
		if (FD_ISSET(socketTable[i], writeSet))
		{
			--selectRtn;
			bSendCheck = SendProcess(socketTable[i]);
		}
		if (FD_ISSET(socketTable[i], readSet))
		{
			--selectRtn;
			if (socketTable[i] == g_ListenSocket)
			{
				AcceptUser();
			}
			else
			{
				if(bSendCheck)
				{
					RecvProcess(socketTable[i]);
					bSendCheck = false;
				}
			}
		}
	}
}

void RecvProcess(SOCKET socket)
{
	Session* curSession = FindSession(socket);
	if (curSession == NULL)
	{
		_LOG(LOG_LEVEL_ERROR, L"FindSession() Error");
		return;
	}

	int directEnQSize = curSession->_RecvRingQ.GetDirectEnqueueSize();
	if (directEnQSize <= 0)
	{
		_LOG(LOG_LEVEL_DEBUG, L"수신 링버퍼 사이즈부족");
		Disconnect(socket);
		return;
	}
	int recvRtn = recv(socket, curSession->_RecvRingQ.GetRearBufferPtr(), directEnQSize,0);

	if (recvRtn <= 0)
	{
		if (recvRtn < 0)
		{
			_LOG(LOG_LEVEL_DEBUG, L"Recv() error:%d", WSAGetLastError());
		}
		Disconnect(socket);
		return;
	}

	curSession->_RecvRingQ.MoveRear(recvRtn);
	


	while (true)
	{
		if (sizeof(st_PACKET_HEADER) > curSession->_RecvRingQ.GetUsedSize())
		{
			break;
		}

		st_PACKET_HEADER header;

		int peekRtn = curSession->_RecvRingQ.Peek((char*)&header, sizeof(st_PACKET_HEADER));

		if (header.wPayloadSize + sizeof(st_PACKET_HEADER) > curSession->_RecvRingQ.GetUsedSize())
		{
			break;
		}
		if (header.byCode != dfPACKET_CODE)
		{
			_LOG(LOG_LEVEL_DEBUG, L"header code not match");
			Disconnect(socket);
			return;
		}
		curSession->_RecvRingQ.MoveFront(sizeof(st_PACKET_HEADER));

		SerializeBuffer packet;

		int deQRtn = curSession->_RecvRingQ.Dequeue(packet.GetBufferPtr(), header.wPayloadSize);

		int moveRtn = packet.MoveWritePos(deQRtn);

		if (deQRtn > moveRtn)
		{
			_LOG(LOG_LEVEL_DEBUG, L"직렬화버퍼에서 뽑고싶은만큼 못뽑았습니다");
		}
		if (header.byCheckSum != GetCheckSum(&packet, header.wMsgType))
		{
			_LOG(LOG_LEVEL_DEBUG, L"체크섬 에러 [Session:%d] ", curSession->_ID);
			Disconnect(socket);
			break;
		}
		

		_LOG(LOG_LEVEL_DEBUG, L"recv[Message:%d]  from Session[%d]", header.wMsgType, curSession->_ID);

		switch (header.wMsgType)
		{
		case df_REQ_LOGIN:
			ProcessPacket_REQ_LOGIN(&packet, curSession);
			break;
		case df_REQ_ROOM_LIST:
			ProcessPacket_REQ_ROOMLIST(curSession);
			break;
		case df_REQ_ROOM_CREATE:
			ProcessPacket_REQ_ROOMCREATE(&packet,curSession);
			break;
		case df_REQ_ROOM_ENTER:
			ProcessPacket_REQ_ROOMENTER(&packet, curSession);
			break;
		case df_REQ_CHAT:
			ProcessPacket_REQ_CHAT(&packet, curSession);
			break;
		case df_REQ_ROOM_LEAVE:
			ProcessPacket_REQ_LEAVE(curSession);
			break;
		}
	}

}

bool SendProcess(SOCKET socket)
{
	Session* curSession = FindSession(socket);
	if (curSession == nullptr)
	{
		_LOG(LOG_LEVEL_ERROR, L"FindSession Fail");
		return false;
	}

	char tempBuffer[5000];

	while (curSession->_SendRingQ.GetUsedSize() != 0)
	{
		int directDeQSize = curSession->_SendRingQ.GetDirectDequeueSize();
		int peekRtn = curSession->_SendRingQ.Peek(tempBuffer, directDeQSize);
		int sendRtn = send(socket, tempBuffer, peekRtn, 0);

		if (sendRtn < 0)
		{
			_LOG(LOG_LEVEL_ERROR, L"Send Fail:%d",WSAGetLastError());
			Disconnect(socket);
			return false;
		}
		curSession->_SendRingQ.MoveFront(sendRtn);
		//_LOG(LOG_LEVEL_DEBUG, L"send :%d[byte] To Session[%d]", sendRtn, curSession->_ID);
	}
	return true;
}
