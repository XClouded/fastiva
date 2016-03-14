#ifndef __FOX_ERRCODE_H__
#define __FOX_ERRCODE_H__

typedef enum {
	FOX_E_OK           =  0,

	// COMMON ERROR-CODES
	FOX_E_PERM         =  1,     /* Operation not permitted */
	FOX_E_NOENT        =  2,     /* No such file or directory */
	FOX_E_SRCH         =  3,     /* No such process */
	FOX_E_INTR         =  4,     /* Interrupted system call */
	FOX_E_IO           =  5,     /* I/O error */
	FOX_E_NXIO         =  6,     /* No such device or address */
	FOX_E_2BIG         =  7,     /* Arg list too long */
	FOX_E_NOEXEC       =  8,     /* Exec format error */
	FOX_E_BADF         =  9,     /* Bad file number */
	FOX_E_CHILD        = 10,     /* No child processes */
	FOX_E_AGAIN        = 11,     /* Try again */
	FOX_E_NOMEM        = 12,     /* Out of memory */
	FOX_E_ACCES        = 13,     /* Permission denied */
	FOX_E_FAULT        = 14,     /* Bad address */

	FOX_E_BUSY         = 16,     /* Device or resource busy */
	FOX_E_EXIST        = 17,     /* File exists */
	FOX_E_XDEV         = 18,     /* Cross-device link */
	FOX_E_NODEV        = 19,     /* No such device */
	FOX_E_NOTDIR       = 20,     /* Not a directory */
	FOX_E_ISDIR        = 21,     /* Is a directory */
	FOX_E_INVAL        = 22,     /* Invalid argument */
	FOX_E_NFILE        = 23,     /* File table overflow */
	FOX_E_MFILE        = 24,     /* Too many open files */
	FOX_E_NOTTY        = 25,     /* Not a typewriter */

	FOX_E_FBIG         = 27,     /* File too large */
	FOX_E_NOSPC        = 28,     /* No space left on device */
	FOX_E_SPIPE        = 29,     /* Illegal seek */
	FOX_E_ROFS         = 30,     /* Read-only file system */
	FOX_E_MLINK        = 31,     /* Too many links */
	FOX_E_PIPE         = 32,     /* Broken pipe */
	FOX_E_DOM          = 33,     /* Math argument out of domain of func */
	FOX_E_RANGE        = 34,     /* Math result not representable */

	// PLATFORM-DEPENDENT ERROR-CODES // linux <asm/errno.h>

	FOX_E_NETDOWN      = 100,    /* Network is down */
	FOX_E_NETUNREACH   = 101,    /* Network is unreachable */
	FOX_E_NETRESET     = 102,    /* Network dropped connection because of reset */
	FOX_E_CONNABORTED  = 103,    /* Software caused connection abort */
	FOX_E_CONNRESET    = 104,    /* Connection reset by peer */
	FOX_E_NOBUFS       = 105,    /* No buffer space available */
	FOX_E_ISCONN       = 106,    /* Transport endpoint is already connected */
	FOX_E_NOTCONN      = 107,    /* Transport endpoint is not connected */
	FOX_E_SHUTDOWN     = 108,    /* Cannot send after transport endpoint shutdown */
    FOX_E_TOOMANYREFS  = 109,    /* Too many references: cannot splice */
	FOX_E_TIMEDOUT     = 110,    /* Connection timed out */
	FOX_E_CONNREFUSED  = 111,    /* Connection refused */
	FOX_E_HOSTDOWN     = 112,    /* Host is down */
	FOX_E_HOSTUNREACH  = 113,    /* No route to host */
	FOX_E_ALREADY      = 114,    /* Operation already in progress */
	FOX_E_INPROGRESS   = 115,    /* Operation now in progress */

	FOX_E_FAIL			= (-1),
	FOX_E_EOF			= (-4),
	FOX_E_WOULDBLOCK	= FOX_E_AGAIN,
} FOX_ERRCODE;


/*=========================================================================*
������ PAL API�� NON-BLOCKING(asynchronous)����� 
�����ϱ� ���� RETURN VALUE TYPE�̴�.
 1. FOX_E_WOULDBLOCK : �Լ��� �����ϱ� ���ؼ� BLOCKING�� �� �� �����Ƿ� 
 	��ɼ����� �����Ͽ����� �ǹ��Ѵ�. OEM-System�� �� �� ��ɼ����� ������ 
 	���°� �Ǹ� �ݵ�� Event�� ���Ͽ� Platform�� �����Ͽ��� �Ѵ�. 
 	Platform�� �ش� Event�� ������ �� �ش� �Լ��� ��ȣ���Ѵ�.
	// ����) sendEvent(), inet_recv/send, serial_recv/send ��
		
 2. FOX_E_IN_PROGRESS : �䱸�� ����� ������ ���� �Ϸ���� �ʾ����� 
    �ǹ��Ѵ�. OEM-System�� ��� ������ �Ϸ�� �� Event�� ���Ͽ� �������� 
    Platform�� �����Ѵ�. �� ��, ��� ���࿡ ������ ��쿡�� �ݵ�� �� ����� 
    �����Ͽ��� �Ѵ�.
	// ����) sendEvent(), ppp_open, inet_connect ��
*-------------------------------------------------------------------------*/

#endif // __FOX_ERRCODE_H__

