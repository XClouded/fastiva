#ifndef __FASTIVA_HEAP__H__
#define __FASTIVA_HEAP__H__

#ifdef __cplusplus
extern "C" {
#endif	

typedef struct {
	int reserved;
}FASTIVA_MEM, * FASTIVA_HMEM;

typedef enum {
	OBJECT_INSTANCE = 0,
	WEAK_REFERENCE = 1,
	NOT_MOVABLE = 2,
	ARRAY_BUFFER = 3,
} FASTIVA_MEM_TYPE;

/** fm::allocMem is used for a more ??? memory allocation features.
	if FASTIVA_MEM_TYPE is 0, Fastiva will create a GC-enabled and
	Movable memory block. If the WEAK_REFERENCE flag is set, the memory
	can be released when the system meory is insufficient.
*/
void  FOX_FASTCALL(fastiva_GC_doGC)(bool fullScan);

// throws Exception when memory insuffiecient.
FASTIVA_HMEM fastiva_GC_malloc(uint siz, FASTIVA_MEM_TYPE type);

// fm::lockMem() can return null when (MEM_TYPE & WEAK_REFERENCE) != 0;
// Then you must call fm::freeMem();
void* FOX_FASTCALL(fastiva_GC_lock)(FASTIVA_HMEM hmem);

void  FOX_FASTCALL(fastiva_GC_unlock)(FASTIVA_HMEM hmem);

void  FOX_FASTCALL(fastiva_GC_free)(FASTIVA_HMEM hmem);

#ifdef __cplusplus
}
#endif	

#endif // __FASTIVA_HEAP__H__