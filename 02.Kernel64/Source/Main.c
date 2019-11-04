#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"


void checkReadWrite();
void Main(void)
{
	int iCursorX, iCursorY;

	kInitializeConsole(0,12);
	kPrintf("Switch To IA-32e Mode Success~!!\n");
	kPrintf("IA-32e C Language Kernel Start..............[Pass]\n");
	kPrintf("This message is printed through the video memory relocated to 0xAB8000\n");

	kGetCursor(&iCursorX, &iCursorY);
	kPrintf("GDT Initialize And Switch For IA-32e Mode...[    ]");
	kInitializeGDTTableAndTSS();
	kLoadGDTR(GDTR_STARTADDRESS);
	kSetCursor(45,iCursorY++);
	kPrintf("Pass\n");

	kPrintf("TSS Segment Load............................[    ]");
	kLoadTR(GDT_TSSSEGMENT);
	kSetCursor(45,iCursorY++);
	kPrintf("Pass\n");

	kPrintf("IDT Initialize..............................[    ]");
	kInitializeIDTTables();
	kLoadIDTR(IDTR_STARTADDRESS);
	kSetCursor(45,iCursorY++);
	kPrintf("Pass\n");

	kPrintf("Total RAM Size check........................[    ]");
	kCheckTotalRAMSize();
	kSetCursor(45, iCursorY++);
	kPrintf("Pass], Size = %d MB\n", kGetTotalRAMSize());

	kPrintf("Keyboard Activate And Queue Initialize......[    ]");
	if(kInitializeKeyboard() == TRUE)
	{
		kSetCursor(45,iCursorY++);
		kPrintf("Pass\n");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else
	{
		kSetCursor(45,iCursorY++);
		kPrintf("Fail\n");
		
		while(1) ;
	}

	kPrintf("PIC Controller And Interrupt Initialize.....[    ]");
	kInitializePIC();
	kMaskPICInterrupt(0);
	kEnableInterrupt();
	kSetCursor(45,iCursorY++);
	kPrintf("Pass\n");

	kStartConsoleShell();
}
