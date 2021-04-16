#include "Log.h"
#include <time.h>

int32_t g_LogLevel=0;   // ��� ���� ����� �α׷���
WCHAR g_LogBuffer[1024]; // �α� ����� �ʿ��� �ӽù���

void Log(WCHAR* string, int32_t logLevel)
{
	time_t curTime;
	tm today;
	time(&curTime);
	localtime_s(&today,&curTime);

	wprintf(L"[%d/%d/%d %d:%d:%d] %s\n",today.tm_year-100,today.tm_mon+1,today.tm_mday,today.tm_hour,today.tm_min,today.tm_sec,string);
}
