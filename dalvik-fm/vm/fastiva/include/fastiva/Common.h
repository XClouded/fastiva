#ifndef __FASTIVA_COMMON_H__
#define __FASTIVA_COMMON_H__
// java/lang/Object.h 에 의해 include 된다.

struct DataObject;
struct InitiatingLoaderList;
struct ClassObject;
struct StringObject;
struct ArrayObject;
struct Method;
struct ExceptionEntry;
struct LineNumEntry;
struct StaticField;
struct InstField;
struct Field;
struct RegisterMap;
struct DvmDex;
struct DexProto;
struct DexFile;

class java_lang_Object;
typedef java_lang_Object Object;

// from DexFile.h
enum PrimitiveType {
    PRIM_NOT        = 0,       /* value is a reference type, not a primitive type */
    PRIM_VOID       = 1,
    PRIM_BOOLEAN    = 2,
    PRIM_BYTE       = 3,
    PRIM_SHORT      = 4,
    PRIM_CHAR       = 5,
    PRIM_INT        = 6,
    PRIM_LONG       = 7,
    PRIM_FLOAT      = 8,
    PRIM_DOUBLE     = 9,
};

enum {
	ACC_FASTIVA_METHOD	=  0x10000000,		// = (1 << 28). 29~31 BIT is used for shor JniInfo.
};

// form DexProto.h
struct DexProto {
    const DexFile* dexFile;     /* file the idx refers to */
	u4 protoIdx;                /* index into proto_ids table of dexFile */
};

// from Common.h
union JValue {
    u1      z;
    s1      b;
    u2      c;
    s2      s;
    s4      i;
    s8      j;
    float   f;
    double  d;
    Object* l;
};

#ifdef __GNUC__
#include <stddef.h>     /* for offsetof() */
#define FASTIVA_OFFSETOF(t, f) offsetof(t, f)
#else
#define FASTIVA_OFFSETOF(t, f)         \
  (reinterpret_cast<char*>(           \
     &reinterpret_cast<t*>(16)->f) -  \
   reinterpret_cast<char*>(16))
#endif

#endif // __FASTIVA_COMMON_H__

