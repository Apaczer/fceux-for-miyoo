// timeStamp.cpp
#include <stdio.h>

#include "timeStamp.h"

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
#endif

#if defined(WIN32)
#include <windows.h>
#endif

//-------------------------------------------------------------------------
//---- Time Stamp Record
//-------------------------------------------------------------------------
#if defined(WIN32)
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#elif defined(__MIPSEL__) || defined(__arm__)
#else
#include <x86intrin.h>
#endif

static uint64_t rdtsc()
{
#if defined(__MIPSEL__) || defined(__arm__)
	// RDTSC is for x86.
	// For MIPS exits register Count(CP0 Register 9,Select 0)
	// but accessing to coprocessor 0 is not possible from userspace (SIGILL)
	// uint32_t tsc;
	// __asm__ __volatile__("mfc0 %0, $9" : "=r"(tsc));
	// printf("TSC: %d\n", tsc);
	// return static_cast<uint64_t>(tsc);
	
	// TSC seems not be used for any relevant task (for now) so we return 0.
	return 0;
#else
	return __rdtsc();
#endif
}

namespace FCEU
{

uint64_t timeStampRecord::_tscFreq = 0;
#if defined(WIN32)
uint64_t timeStampRecord::qpcFreq = 0;
#endif

void timeStampRecord::readNew(void)
{
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
	clock_gettime( CLOCK_REALTIME, &ts );
#else
	QueryPerformanceCounter((LARGE_INTEGER*)&ts);
#endif
	tsc = rdtsc();
}
#if defined(WIN32)
void timeStampRecord::qpcCalibrate(void)
{
		if (QueryPerformanceFrequency((LARGE_INTEGER*)&timeStampRecord::qpcFreq) == 0)
		{
			printf("QueryPerformanceFrequency FAILED!\n");
		}
}
#endif

class timeStampModule
{
	public:
	timeStampModule(void)
	{
		printf("timeStampModuleInit\n");
	#if defined(WIN32)
		timeStampRecord::qpcCalibrate();
	#endif
	}
};

static timeStampModule module;

bool timeStampModuleInitialized(void)
{
#if defined(WIN32)
	bool initialized = timeStampRecord::countFreq() != 0;
#else
	bool initialized = true;
#endif
	return initialized;
}

void timeStampRecord::tscCalibrate(int numSamples)
{
	timeStampRecord t1, t2, td;
	uint64_t td_sum = 0;
	double td_avg;

#if defined(WIN32)
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&timeStampRecord::qpcFreq) == 0)
	{
		printf("QueryPerformanceFrequency FAILED!\n");
	}
#endif
	printf("Running TSC Calibration: %i sec...\n", numSamples);

	for (int i=0; i<numSamples; i++)
	{
		t1.readNew();
#if defined(WIN32)
		Sleep(1000);
#else
		sleep(1);
#endif
		t2.readNew();

		td += t2 - t1;

		td_sum = td.tsc;

		td_avg = static_cast<double>(td_sum);

		timeStampRecord::_tscFreq = static_cast<uint64_t>( td_avg / td.toSeconds() );

		printf("%i Calibration: %f sec   TSC:%llu   TSC Freq: %f MHz\n", i, td.toSeconds(), 
			static_cast<unsigned long long>(td.tsc), static_cast<double>(timeStampRecord::_tscFreq) * 1.0e-6 );
	}
}

} // namespace FCEU
