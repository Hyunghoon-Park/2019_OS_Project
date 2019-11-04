#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"

void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };

    vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 1 ] = '0' + iVectorNumber % 10;

	if(iVectorNumber == 14)
	{
		int iX, iY;
		long *ptr = (long*)0x142000;
		void *temp = (void*)0x142ff8;
		if((qwErrorCode & 1) == 0)
		{
			kGetCursor(&iX, &iY);
			kSetCursor(0, iY);

    		kPrintf("====================================================\n" );
    		kPrintf("                 Page fault Occur~!!!!               \n" );
    		kPrintf("                    Vector: 0x1ff000                 \n" );
    		kPrintf("====================================================\n" );
			//kSetCursor(7, iY + 4);
			ptr[511] = ptr[511] | 1;
			invlpg(temp);
		}
		else if((qwErrorCode & 2) == 2)
		{
			kGetCursor(&iX, &iY);
			kSetCursor(0, iY);
    		kPrintf("====================================================\n" );
    		kPrintf("                 Protection fault Occur~!!!!         \n" );
    		kPrintf("                    Vector: 0x1ff000                 \n" );
    		kPrintf("====================================================\n" );
			//kSetCursor(7, iY + 4);
			ptr[511] = ptr[511] | 2;
		}
	}
	else{
    	kPrintStringXY( 0, 0, "====================================================" );
    	kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    	kPrintStringXY( 0, 2, "                    Vector:                         " );
    	kPrintStringXY( 27, 2, vcBuffer );
    	kPrintStringXY( 0, 3, "====================================================" );
    	while( 1 ) ;
	}

}

void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }

    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

static inline void invlpg(void* m){
	asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}
