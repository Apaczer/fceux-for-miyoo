// utilities.cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <libgen.h>

//---------------------------------------------------------------------------
//  Returns the path of fceux.exe as a string.
int fceuExecutablePath( char *outputPath, int outputSize )
{
	if ( (outputPath == NULL) || (outputSize <= 0) )
	{
		return -1;
	}
	outputPath[0] = 0;

	char exePath[ 2048 ];
	ssize_t count = ::readlink( "/proc/self/exe", exePath, sizeof(exePath)-1 );

	if ( count > 0 )
	{
		char *dir;
		exePath[count] = 0;
		//printf("EXE Path: '%s' \n", exePath );

		dir = ::dirname( exePath );

		if ( dir )
		{
			//printf("DIR Path: '%s' \n", dir );
			strncpy( outputPath, dir, outputSize );
			outputPath[outputSize-1] = 0;
			return 0;
		}
	}

	return -1;
}
