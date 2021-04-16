#include "SelectServer.h"

int main()
{
	Init_Network();
	
	while (true)
	{
		NetworkProcess();

	}
	return 0;
}
