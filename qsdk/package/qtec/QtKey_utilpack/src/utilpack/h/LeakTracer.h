#ifndef LEAKTRACER_H
#define LEAKTRACER_H

#if 0
#ifdef _STDLIB_H
  #error can not include stdlib.h before this file.
#endif // _STDLIB_H

#define __need_malloc_and_calloc
#include <stdlib.h>
#define __need_malloc_and_calloc

extern void *calloc(size_t nmemb, size_t size);
extern void *malloc(size_t size);
extern void free(void *ptr);
extern void *realloc(void *ptr, size_t size);
#endif

#define THREAD_SAVE
#define _THREAD_SAFE


#ifdef THREAD_SAVE
#define _THREAD_SAVE
#include <pthread.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

class LeakTracer 
{
public:
	static LeakTracer* Instance();
	
	void DumpAllToFile();
	
private:
	int m_nFileCount;
		
private:
	struct Leak {
		const void *addr;
		size_t      size;
		const void *allocAddr;
		bool        type;
		int         nextBucket;
	};
	
	int  newCount;      // how many memory blocks do we have
	int  leaksCount;    // amount of entries in the leaks array
	int  firstFreeSpot; // Where is the first free spot in the leaks array?
	int  currentAllocated; // currentAllocatedMemory
	int  maxAllocated;     // maximum Allocated
	unsigned long totalAllocations; // total number of allocations. stats.
	unsigned int  abortOn;  // resons to abort program (see abortReason_t)

	/**
	 * Have we been initialized yet?  We depend on this being
	 * false before constructor has been called!  
	 */
	bool initialized;	
	bool destroyed;		// Has our destructor been called?


	FILE *report;       // filedescriptor to write to

	/**
	 * pre-allocated array of leak info structs.
	 */
	Leak *leaks;

	/**
	 * fast hash to lookup the spot where an allocation is 
	 * stored in case of an delete. map<void*,index-in-leaks-array>
	 */
	int  *leakHash;     // fast lookup

#ifdef THREAD_SAVE
	pthread_mutex_t mutex;
#endif

	enum abortReason_t {
		OVERWRITE_MEMORY    = 0x01,
		DELETE_NONEXISTENT  = 0x02,
		NEW_DELETE_MISMATCH = 0x04
	};

public:
	LeakTracer();
	
	~LeakTracer();
	
	void initialize();
	
	/*
	 * the workhorses:
	 */
	void *registerAlloc(size_t size, bool type);
	void  registerFree (void *p, bool type);

	/**
	 * write a hexdump of the given area.
	 */
	void  hexdump(const unsigned char* area, int size);
	
	/**
	 * Terminate current running progam.
	 */
	void progAbort(abortReason_t reason);

	/**
	 * write a Report over leaks, e.g. still pending deletes
	 */
	void writeLeakReport();
	
	void WriteLeakReport_i(FILE *aFile);
};

#endif // LEAKTRACER_H
