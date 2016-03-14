#ifndef __FOX_CONDITION_H__
#define __FOX_CONDITION_H__

#include <fox/Config.h>

#ifdef __cplusplus
extern "C" {
#endif	

struct fox_Condition;

fox_Condition* FOX_FASTCALL(fox_condition_create)(FOX_BOOL initialState);

FOX_BOOL FOX_FASTCALL(fox_condition_isLocked)(fox_Condition* hCondition);

FOX_BOOL FOX_FASTCALL(fox_condition_wait)(fox_Condition* hCondition);

FOX_BOOL FOX_FASTCALL(fox_condition_wake)(fox_Condition* hCondition, int cntThread);

FOX_BOOL FOX_FASTCALL(fox_condition_destroy)(fox_Condition* hCondition);


#ifdef __cplusplus
}
#endif	

#endif // __FOX_CONDITION_H__
