#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"

void kCommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode);
void kCommonInterruptHandler(int iVectorNumber);
void kKeyboardHandler(int iVectorNumber);
static inline void invlpg(void* m);
void kTimerHandler(int iVectorNumber);
void kPageFault(int p, QWORD qwErrorCode);
<<<<<<< HEAD
void kHDDHandler(int iVectorNumber);
=======
void kHDDHandler( int iVectorNumber );
>>>>>>> NEC

#endif
