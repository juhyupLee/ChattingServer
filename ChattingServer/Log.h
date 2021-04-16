#pragma once
#include <iostream>
#include <Windows.h>

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_ERROR 2


extern int32_t g_LogLevel;   // 출력 저장 대상의 로그레벨
extern WCHAR g_LogBuffer[1024]; // 로그 저장시 필요한 임시버퍼


void Log(WCHAR* string, int32_t logLevel);

#define _LOG(LogLevel,fmt,...)					\
do{												\
	if(g_LogLevel<= LogLevel)					\
	{											\
		wsprintf(g_LogBuffer,fmt,##__VA_ARGS__);\
		Log(g_LogBuffer,LogLevel);				\
	}											\
}while(0)										\


