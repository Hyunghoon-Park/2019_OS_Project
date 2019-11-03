#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"

SHELLCOMMANDENTRY gs_vstCommandTable[] =
    {
        {"help", "Show Help", kHelp},
        {"cls", "Clear Screen", kCls},
        {"totalram", "Show Total RAM Size", kShowTotalRAMSize},
        {"strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest},
        {"shutdown", "Shutdown And Reboot OS", kShutdown},
        {"hellow", "Welcome OS", kHellow},       //Dummy
        {"student", "We are student", kStudent}, //Dummy
};

void kStartConsoleShell(void)
{
    char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;
    int iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
    int tabNum = 0;
    int updownIndex = 0;
    char historyList[11][CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
    historyList[0][0] = '\0';

    kPrintf(CONSOLESHELL_PROMPTMESSAGE);

    while (1)
    {
        bKey = kGetCh();
        if (bKey == KEY_BACKSPACE)
        {
            if (iCommandBufferIndex > 0)
            {
                kGetCursor(&iCursorX, &iCursorY);
                kPrintStringXY(iCursorX - 1, iCursorY, " ");
                kSetCursor(iCursorX - 1, iCursorY);
                vcCommandBuffer[--iCommandBufferIndex] = '\0'; //buffer set to NULL
                tabNum = 0;
            }
        }
        else if (bKey == KEY_ENTER)
        {
            kPrintf("\n");

            if (iCommandBufferIndex > 0)
            {
                for (int i = 9; i >= 1; i--)
                {
                    kMemSet(historyList[i + 1], '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
                    kMemCpy(historyList[i + 1], historyList[i], kStrLen(historyList[i]));
                    kMemSet(historyList[i], '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
                }                
                kMemCpy(historyList[1], vcCommandBuffer, iCommandBufferIndex);

                vcCommandBuffer[iCommandBufferIndex] = '\0';
                kExecuteCommand(vcCommandBuffer);
            }

            kPrintf("%s", CONSOLESHELL_PROMPTMESSAGE);
            kMemSet(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
            iCommandBufferIndex = 0;
            tabNum = 0;      //tabNum
            updownIndex = 0; //reset history index
        }
        else if ((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) ||
                 (bKey == KEY_CAPSLOCK) || (bKey == KEY_NUMLOCK) ||
                 (bKey == KEY_SCROLLLOCK))
        {
            ;
        }
        else
        {
            int totalCount = 0, i, j;
            char *cmdList[iCount], *shortCmd = '\0';
            BOOL check = FALSE;

            //Tab Shortcut Key
            if (bKey == KEY_TAB)
            {
                if (iCommandBufferIndex > 0)
                {
                    tabNum++;

                    int oldLen = iCommandBufferIndex;

                    for (int i = 0; i < iCount; i++)
                    {
                        if (kStrnCmp(vcCommandBuffer, gs_vstCommandTable[i].pcCommand, iCommandBufferIndex) == 1)
                        {
                            cmdList[totalCount++] = gs_vstCommandTable[i].pcCommand;
                            if (shortCmd == '\0' || kStrLen(shortCmd) > kStrLen(gs_vstCommandTable[i].pcCommand))
                                shortCmd = gs_vstCommandTable[i].pcCommand;
                        }
                    }

                    for (i = iCommandBufferIndex; i < kStrLen(shortCmd); i++)
                    {
                        for (j = 0; j < totalCount; j++)
                        {
                            if (cmdList[j][i] == shortCmd[i])
                                check = TRUE;
                            else
                            {
                                check = FALSE;
                                break;
                            }
                        }
                        if (check == FALSE)
                            break;
                    }

                    if (i == kStrLen(shortCmd))
                        tabNum = 0;

                    for (j = iCommandBufferIndex; j < i; j++)
                    {
                        vcCommandBuffer[iCommandBufferIndex++] = shortCmd[j];
                    }

                    kGetCursor(&iCursorX, &iCursorY);
                    kPrintStringXY(7, iCursorY, vcCommandBuffer);
                    kSetCursor(7 + kStrLen(vcCommandBuffer), iCursorY);

                    if (tabNum > 1 && totalCount != 0)
                    {
                        kPrintf("\n");
                        for (int i = 0; i < totalCount; i++)
                        {
                            kPrintf(cmdList[i]);
                            kPrintf("  ");
                        }
                        kPrintf("\n");
                        kPrintf("%s", CONSOLESHELL_PROMPTMESSAGE);
                        kPrintf(vcCommandBuffer);
                    }
                }
            }
            else if ((bKey == KEY_UP) || (bKey == KEY_DOWN))
            {
                while (iCommandBufferIndex)
                {
                    kGetCursor(&iCursorX, &iCursorY);
                    kPrintStringXY(iCursorX - 1, iCursorY, " ");
                    kSetCursor(iCursorX - 1, iCursorY);
                    vcCommandBuffer[--iCommandBufferIndex] = '\0'; //buffer set to NULL
                }
                kGetCursor(&iCursorX, &iCursorY);
                if (bKey == KEY_UP)
                {
                    updownIndex++;
                    if (updownIndex > 10)
                    {
                        updownIndex = 10;
                    }
                    if(historyList[updownIndex][0] == '\0')
                    updownIndex--;
                }
                else if(bKey == KEY_DOWN)
                {
                    updownIndex--;
                    if (updownIndex < 0)
                    {
                        updownIndex = 0;
                    }
                }
                kMemCpy(vcCommandBuffer, historyList[updownIndex], kStrLen(historyList[updownIndex]));
                kPrintStringXY(iCursorX, iCursorY, vcCommandBuffer);
                iCommandBufferIndex = kStrLen(vcCommandBuffer);
                kSetCursor(iCursorX + iCommandBufferIndex, iCursorY);
            }
            else
            {
                if (iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT)
                {
                    vcCommandBuffer[iCommandBufferIndex++] = bKey;
                    kPrintf("%c", bKey);
                }
            }
        }
    }
}
void kExecuteCommand(const char *pcCommandBuffer)
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;

    iCommandBufferLength = kStrLen(pcCommandBuffer);
    for (iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++)
    {
        if (pcCommandBuffer[iSpaceIndex] == ' ')
        {
            break;
        }
    }

    iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
    for (i = 0; i < iCount; i++)
    {
        iCommandLength = kStrLen(gs_vstCommandTable[i].pcCommand);
        if ((iCommandLength == iSpaceIndex) &&
            (kMemCmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer,
                     iSpaceIndex) == 0))
        {
            gs_vstCommandTable[i].pfFunction(pcCommandBuffer + iSpaceIndex + 1);
            break;
        }
    }

    if (i >= iCount)
    {
        kPrintf("'%s' is not found.\n", pcCommandBuffer);
    }
}

void kInitializeParameter(PARAMETERLIST *pstList, const char *pcParameter)
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen(pcParameter);
    pstList->iCurrentPosition = 0;
}

int kGetNextParameter(PARAMETERLIST *pstList, char *pcParameter)
{
    int i;
    int iLength;

    if (pstList->iLength <= pstList->iCurrentPosition)
    {
        return 0;
    }

    for (i = pstList->iCurrentPosition; i < pstList->iLength; i++)
    {
        if (pstList->pcBuffer[i] == ' ')
        {
            break;
        }
    }

    kMemCpy(pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i);
    iLength = i - pstList->iCurrentPosition;
    pcParameter[iLength] = '\0';

    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}

void kHelp(const char *pcCommandBuffer)
{
    int i;
    int iCount;
    int iCursorX, iCursorY;
    int iLength, iMaxCommandLength = 0;

    kPrintf("=========================================================\n");
    kPrintf("                    MINT64 Shell Help                    \n");
    kPrintf("=========================================================\n");

    iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

    for (i = 0; i < iCount; i++)
    {
        iLength = kStrLen(gs_vstCommandTable[i].pcCommand);
        if (iLength > iMaxCommandLength)
        {
            iMaxCommandLength = iLength;
        }
    }

    for (i = 0; i < iCount; i++)
    {
        kPrintf("%s", gs_vstCommandTable[i].pcCommand);
        kGetCursor(&iCursorX, &iCursorY);
        kSetCursor(iMaxCommandLength, iCursorY);
        kPrintf("  - %s\n", gs_vstCommandTable[i].pcHelp);
    }
}

void kCls(const char *pcParameterBuffer)
{
    kClearScreen();
    kSetCursor(0, 1);
}

void kShowTotalRAMSize(const char *pcParameterBuffer)
{
    kPrintf("Total RAM Size = %d MB\n", kGetTotalRAMSize());
}

void kStringToDecimalHexTest(const char *pcParameterBuffer)
{
    char vcParameter[100];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;

    kInitializeParameter(&stList, pcParameterBuffer);

    while (1)
    {
        iLength = kGetNextParameter(&stList, vcParameter);
        if (iLength == 0)
        {
            break;
        }

        kPrintf("Param %d = '%s', Length = %d, ", iCount + 1,
                vcParameter, iLength);

        if (kMemCmp(vcParameter, "0x", 2) == 0)
        {
            lValue = kAToI(vcParameter + 2, 16);
            kPrintf("HEX Value = %q\n", lValue);
        }
        else
        {
            lValue = kAToI(vcParameter, 10);
            kPrintf("Decimal Value = %d\n", lValue);
        }

        iCount++;
    }
}

void kShutdown(const char *pcParameterBuffer)
{
    kPrintf("System Shutdown Start...\n");
    kPrintf("Press Any Key To Reboot PC...");
    kGetCh();
    kReboot();
}

//Dummy Function
void kHellow(const char *pcParameterBuffer)
{
    kPrintf("Hellow World!\n");
}

void kStudent(const char *pcParameterBuffer)
{
    kPrintf("We are Student!\n");
}
