#ifndef __FASTIVA_PREPROCESS__UTIL_H__
#define __FASTIVA_PREPROCESS__UTIL_H__


#define FASTIVA_COMMENT(msg)				// ignore

#define FASTIVA_MAKE_STR(S)					#S


#define FASTIVA_MERGE_TOKEN_EX(L, R)		L##R

#define FASTIVA_MERGE_TOKEN(L, R)			FASTIVA_MERGE_TOKEN_EX(L, R)

#define FASTIVA_MERGE_3TOKEN_EX(L, M, R)	L##M##R

#define FASTIVA_MERGE_3TOKEN(L, M, R)		FASTIVA_MERGE_3TOKEN_EX(L, M, R)

#define FASTIVA_ARRAY_ITEM_COUNT(array)		(sizeof(array) / sizeof(array[0]))

#endif // __FASTIVA_PREPROCESS__UTIL_H__
