/* 
 * Homepage: <http://www.andreasen.org/LeakTracer/>
 *
 * Authors:
 *  Erwin S. Andreasen <erwin@andreasen.org>
 *  Henner Zeller <H.Zeller@acm.org>
 *
 * This program is Public Domain
 */

#include "LeakTracer.h"

#undef malloc
#undef realloc
#undef free
#undef calloc

/*
 * underlying allocation, de-allocation used within 
 * this tool
 */
#define LT_MALLOC  malloc
#define LT_FREE    free
#define LT_REALLOC realloc

/*
 * prime number for the address lookup hash table.
 * if you have _really_ many memory allocations, use a
 * higher value, like 343051 for instance.
 */
#define SOME_PRIME 35323
#define ADDR_HASH(addr) ((unsigned long) addr % SOME_PRIME)

/**
 * Filedescriptor to write to. This should not be a low number,
 * because these often have special meanings (stdin, out, err)
 * and may be closed by the program (daemons)
 * So choose an arbitrary higher FileDescriptor .. e.g. 42
 */
#define FILEDESC    42

/**
 * allocate a bit more memory in order to check if there is a memory
 * overwrite. Either 0 or more than sizeof(unsigned int). Note, you can
 * only detect memory over_write_, not _reading_ beyond the boundaries. Better
 * use electric fence for these kind of bugs 
 *   <ftp://ftp.perens.com/pub/ElectricFence/>
 */
#define MAGIC "\xAA\xBB\xCC\xDD"
#define MAGIC_SIZE (sizeof(MAGIC)-1)

/**
 * on 'new', initialize the memory with this value.
 * if not defined - uninitialized. This is very helpful because
 * it detects if you initialize your classes correctly .. if not,
 * this helps you faster to get the segmentation fault you're 
 * implicitly asking for :-). 
 *
 * Set this to some value which is likely to produce a
 * segmentation fault on your platform.
 */
#define SAVEVALUE   0xAA

/**
 * on 'delete', clean memory with this value.
 * if not defined - no memory clean.
 *
 * Set this to some value which is likely to produce a
 * segmentation fault on your platform.
 */
#define MEMCLEAN    0xEE

/**
 * Initial Number of memory allocations in our list.
 * Doubles for each re-allocation.
 */
#define INITIALSIZE 32768

static LeakTracer leakTracer;

LeakTracer* LeakTracer::Instance()
{
	return &leakTracer;
}

void LeakTracer::DumpAllToFile()
{
	char szFileName[256];
	sprintf(szFileName, "./DumpLeakTracer_%d.out", ++m_nFileCount);
	FILE *pFile = fopen(szFileName, "w+");
	if (!pFile) {
		fprintf(stderr, "LeakTracer: cannot open dump file %s.\n",	szFileName);
		return;
	}
	WriteLeakReport_i(pFile);
	fclose(pFile);
}

	LeakTracer::LeakTracer() {
		initialize();
	}
	
	void LeakTracer:: initialize() {
		// Unfortunately we might be called before our constructor has actualy fired
		if (initialized)
			return;

		//		fprintf(stderr, "LeakTracer::initialize()\n");
		m_nFileCount = 0;
		initialized = true;
		newCount = 0;
		leaksCount = 0;
		firstFreeSpot = 1; // index '0' is special
		currentAllocated = 0;
		maxAllocated = 0;
		totalAllocations = 0;
		abortOn =  OVERWRITE_MEMORY | DELETE_NONEXISTENT | NEW_DELETE_MISMATCH; // only _severe_ reason
		report = 0;
		leaks = 0;
		leakHash = 0;

		char uniqFilename[256];
		const char *filename = getenv("LEAKTRACE_FILE") ? : "leak.out";
		struct stat dummy;
		if (stat(filename, &dummy) == 0) {
			sprintf(uniqFilename, "%s.%d", filename, getpid());
			fprintf(stderr, 
				"LeakTracer: file exists; using %s instead\n",
				uniqFilename);
		}
		else {
			sprintf(uniqFilename, "%s", filename);
		}
		int reportfd = open(uniqFilename, 
				    O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
		if (reportfd < 0) {
			fprintf(stderr, "LeakTracer: cannot open %s: %m\n", 
				filename);
			report = stderr;
		}
		else {
			int dupfd = dup2(reportfd, FILEDESC);
			close(reportfd);
			report = fdopen(dupfd, "w");
			if (report == NULL) {
				report = stderr;
			}
		}
		
		time_t t = time(NULL);
		fprintf (report, "# starting %s", ctime(&t));

		leakHash = (int*) LT_MALLOC(SOME_PRIME * sizeof(int));
		memset ((void*) leakHash, 0x00, SOME_PRIME * sizeof(int));

#ifdef MAGIC
		fprintf (report, "# memory overrun protection of %d Bytes\n", MAGIC_SIZE);
#endif
		
#ifdef SAVEVALUE
		fprintf (report, "# initializing new memory with 0x%2X\n",
			 SAVEVALUE);
#endif

#ifdef MEMCLEAN
		fprintf (report, "# sweeping deleted memory with 0x%2X\n",
			 MEMCLEAN);
#endif
		if (getenv("LT_ABORTREASON")) {
			abortOn = atoi(getenv("LT_ABORTREASON"));
		}

#define PRINTREASON(x) if (abortOn & x) fprintf(report, "%s ", #x);
		fprintf (report, "# aborts on ");
		PRINTREASON( OVERWRITE_MEMORY );
		PRINTREASON( DELETE_NONEXISTENT );
		PRINTREASON( NEW_DELETE_MISMATCH );
		fprintf (report, "\n");
#undef PRINTREASON

#ifdef THREAD_SAVE
		fprintf (report, "# thread save\n");
		/*
		 * create default, non-recursive ('fast') mutex
		 * to lock our datastructure where we keep track of
		 * the user's new/deletes
		 */
		if (pthread_mutex_init(&mutex, NULL) < 0) {
			fprintf(report, "# couldn't init mutex ..\n");
			fclose(report);
			_exit(1);
		}
#else
		fprintf(report, "# not thread save; if you use threads, recompile with -DTHREAD_SAVE\n");
#endif
		fflush(report);
	}
	
	/**
	 * Terminate current running progam.
	 */
	void LeakTracer::progAbort(abortReason_t reason) {
		if (abortOn & reason) {
			fprintf(report, "# abort; DUMP of current state\n");
                        fprintf(stderr, "LeakTracer aborting program\n");
			writeLeakReport();
			fclose(report);
			abort();
		}
		else
			fflush(report);
	}

	LeakTracer::~LeakTracer() {
	    //		fprintf(stderr, "LeakTracer::destroy()\n");
		time_t t = time(NULL);
		fprintf (report, "# finished %s", ctime(&t));
		writeLeakReport();
		fclose(report);
		free(leaks);
#ifdef THREAD_SAVE
		pthread_mutex_destroy(&mutex);
#endif
		destroyed = true;
	}
 

void* LeakTracer::registerAlloc (size_t size, bool type) {
	initialize();

	//	fprintf(stderr, "LeakTracer::registerAlloc()\n");

	if (destroyed) {
		fprintf(stderr, "Oops, registerAlloc called after destruction of LeakTracer (size=%d)\n", size);
		return LT_MALLOC(size);
	}


	void *p = LT_MALLOC(size + MAGIC_SIZE);
	// Need to call the new-handler
	if (!p) {
		fprintf(report, "LeakTracer malloc %m\n");
		_exit (1);
	}

#ifdef SAVEVALUE
	/* initialize with some defined pattern */
	memset(p, SAVEVALUE, size + MAGIC_SIZE);
#endif
	
#ifdef MAGIC
	/*
	 * the magic value is a special pattern which does not need
	 * to be uniform.
	 */
        memcpy((char*)p+size, MAGIC, MAGIC_SIZE);
#endif

#ifdef THREAD_SAVE
	pthread_mutex_lock(&mutex);
#endif

	++newCount;
	++totalAllocations;
	currentAllocated += size;
	if (currentAllocated > maxAllocated)
		maxAllocated = currentAllocated;
	
	for (;;) {
		for (int i = firstFreeSpot; i < leaksCount; i++)
			if (leaks[i].addr == NULL) {
				leaks[i].addr = p;
				leaks[i].size = size;
				leaks[i].type = type;
				leaks[i].allocAddr=__builtin_return_address(1);
				firstFreeSpot = i+1;
				// allow to lookup our index fast.
				int *hashPos = &leakHash[ ADDR_HASH(p) ];
				leaks[i].nextBucket = *hashPos;
				*hashPos = i;
#ifdef THREAD_SAVE
				pthread_mutex_unlock(&mutex);
#endif
				return p;
			}
		
		// Allocate a bigger array
		// Note that leaksCount starts out at 0.
		int new_leaksCount = (leaksCount == 0) ? INITIALSIZE 
			                               : leaksCount * 2;
		leaks = (Leak*)LT_REALLOC(leaks, 
					  sizeof(Leak) * new_leaksCount);
		if (!leaks) {
			fprintf(report, "# LeakTracer realloc failed: %m\n");
			_exit(1);
		}
		else {
			fprintf(report, "# internal buffer now %d\n", 
				new_leaksCount);
			fflush(report);
		}
		memset(leaks+leaksCount, 0x00,
		       sizeof(Leak) * (new_leaksCount-leaksCount));
		leaksCount = new_leaksCount;
	}
}

void LeakTracer::hexdump(const unsigned char* area, int size) {
	fprintf(report, "# ");
	for (int j=0; j < size ; ++j) {
		fprintf (report, "%02x ", *(area+j));
		if (j % 16 == 15) {
			fprintf(report, "  ");
			for (int k=-15; k < 0 ; k++) {
				char c = (char) *(area + j + k);
				fprintf (report, "%c", isprint(c) ? c : '.');
			}
			fprintf(report, "\n# ");
		}
	}
	fprintf(report, "\n");
}

void LeakTracer::registerFree (void *p, bool type) {
	initialize();

	if (p == NULL)
		return;

	if (destroyed) {
		fprintf(stderr, "Oops, allocation destruction of LeakTracer (p=%p)\n", p);
		return;
	}

#ifdef THREAD_SAVE
	pthread_mutex_lock(&mutex);
#endif
	int *lastPointer = &leakHash[ ADDR_HASH(p) ];
	int i = *lastPointer;

	while (i != 0 && leaks[i].addr != p) {
		lastPointer = &leaks[i].nextBucket;
		i = *lastPointer;
	}

	if (leaks[i].addr == p) {
		*lastPointer = leaks[i].nextBucket; // detach.
		newCount--;
		leaks[i].addr = NULL;
		currentAllocated -= leaks[i].size;
		if (i < firstFreeSpot)
			firstFreeSpot = i;

		if (leaks[i].type != type) {
			fprintf(report, 
				"S %10p %10p  # new%s but delete%s "
				"; size %d\n",
				leaks[i].allocAddr,
				__builtin_return_address(1),
				((!type) ? "[]" : " normal"),
				((type) ? "[]" : " normal"),
				leaks[i].size);
			
			//progAbort( NEW_DELETE_MISMATCH );
		}
#ifdef MAGIC
		if (memcmp((char*)p + leaks[i].size, MAGIC, MAGIC_SIZE)) {
			fprintf(report, "O %10p %10p  "
				"# memory overwritten beyond allocated"
				" %d bytes\n",
				leaks[i].allocAddr,
				__builtin_return_address(1),
				leaks[i].size);
			fprintf(report, "# %d byte beyond area:\n",
				MAGIC_SIZE);
			hexdump((unsigned char*)p+leaks[i].size,
				MAGIC_SIZE);
			//progAbort( OVERWRITE_MEMORY );
		}
#endif

#ifdef THREAD_SAVE
#  ifdef MEMCLEAN
		int allocationSize = leaks[i].size;
#  endif
		pthread_mutex_unlock(&mutex);
#else
#define             allocationSize leaks[i].size
#endif

#ifdef MEMCLEAN
		// set it to some garbage value.
		memset((unsigned char*)p, MEMCLEAN, allocationSize + MAGIC_SIZE);
#endif
		LT_FREE(p);
		return;
	}

#ifdef THREAD_SAVE
	pthread_mutex_unlock(&mutex);
#endif
	fprintf(report, "D %10p             # delete non alloc or twice pointer %10p\n", 
		__builtin_return_address(1), p);
	//progAbort( DELETE_NONEXISTENT );
}


void LeakTracer::writeLeakReport() {
	initialize();
	WriteLeakReport_i(report);
}

void LeakTracer::WriteLeakReport_i(FILE *aFile)
{
#ifdef THREAD_SAVE
		pthread_mutex_lock(&mutex);
#endif

	if (newCount > 0) {
		fprintf(aFile, "# LeakReport\n");
		fprintf(aFile, "# %10s | %9s  # Pointer Addr\n",
			"from new @", "size");
	}
	for (int i = 0; i <  leaksCount; i++)
		if (leaks[i].addr != NULL) {
			// This ought to be 64-bit safe?
			fprintf(aFile, "L %10p   %9ld  # %p\n",
				leaks[i].allocAddr,
				(long) leaks[i].size,
				leaks[i].addr);
		}
	fprintf(aFile, "# total allocation requests: %6ld ; max. mem used"
		" %d kBytes\n", totalAllocations, maxAllocated / 1024);
	fprintf(aFile, "# leak %6d Bytes\t:-%c\n", currentAllocated,
		(currentAllocated == 0) ? ')' : '(');
	if (currentAllocated > 50 * 1024) {
		fprintf(aFile, "# .. that is %d kByte!! A lot ..\n", 
			currentAllocated / 1024);
	}
	
#ifdef THREAD_SAVE
		pthread_mutex_unlock(&mutex);
#endif
}

/** -- The actual new/delete operators -- **/

void* operator new(size_t size) {
	return leakTracer.registerAlloc(size,false);
}


void* operator new[] (size_t size) {
	return leakTracer.registerAlloc(size,true);
}


void operator delete (void *p) {
	leakTracer.registerFree(p,false);
}


void operator delete[] (void *p) {
	leakTracer.registerFree(p,true);
}

/* Emacs: 
 * Local variables:
 * c-basic-offset: 8
 * End:
 * vi:set tabstop=8 shiftwidth=8 nowrap: 
 */
