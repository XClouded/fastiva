#ifndef WIN32_TIB_FOR_FOX
#define WIN32_TIB_FOR_FOX


//===========================================================
// File: TIB.H
// Author: Matt Pietrek
// From: Microsoft Systems Journal "Under the Hood", May 1996
//===========================================================
#pragma pack(1)

typedef struct _EXCEPTION_REGISTRATION_RECORD
{
    struct _EXCEPTION_REGISTRATION_RECORD * pNext;
    void (__stdcall *pfnHandler)();
} EXCEPTION_REGISTRATION_RECORD, *PEXCEPTION_REGISTRATION_RECORD;

typedef struct TIB_WIN32
{
	PEXCEPTION_REGISTRATION_RECORD pvExcept; // 00h Head of exception record list
	void*   pvStackUserTop;     // 04h Top of user stack
	void*   pvStackUserBase;    // 08h Base of user stack

	union                       // 0Ch (NT/Win95 differences)
	{
		struct  // Win95 fields
		{
			unsigned short  pvTDB;         // 0Ch TDB
			unsigned short  pvThunkSS;     // 0Eh SS selector used for thunking to 16 bits
			unsigned int	unknown1;      // 10h
		} WIN95;

		struct  // WinNT fields
		{
			void* SubSystemTib;     // 0Ch
			unsigned int FiberData;        // 10h
		} WINNT;
	} TIB_UNION1;

	void*   pvArbitrary;        // 14h Available for application use
	struct _tib *ptibSelf;      // 18h Linear address of TIB structure

	union                       // 1Ch (NT/Win95 differences)
	{
		struct  // Win95 fields
		{
			unsigned short    TIBFlags;           // 1Ch
			unsigned short    Win16MutexCount;    // 1Eh
			unsigned int   DebugContext;       // 20h
			unsigned int   pCurrentPriority;   // 24h
			unsigned int   pvQueue;            // 28h Message Queue selector
		} WIN95;

		struct  // WinNT fields
		{
			unsigned int unknown1;             // 1Ch
			unsigned int processID;            // 20h
			unsigned int threadID;             // 24h
			unsigned int unknown2;             // 28h
		} WINNT;
	} TIB_UNION2;

	void**  pvTLSArray;         // 2Ch Thread Local Storage array

	union                       // 30h (NT/Win95 differences)
	{
		struct  // Win95 fields
		{
			void**  pProcess;     // 30h Pointer to owning process database
		} WIN95;
	} TIB_UNION3; 
} TIB_WIN32, *PTIB;


#pragma pack()
#endif// WIN32_TIB_FOR_FOX