#include "PSPWrapper.h"

#include <windows.h>

int main(int argc, char *argv[])
{
	CPSPWrapper psp;

	if(false == psp.Initialize("PSPRadio_AllStates.theme"))
	{
		printf("Error initializing PSPWrapper\n");
	}

	psp.Run();

	return 1;
}