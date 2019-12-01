<<<<<<< HEAD
/**
 *  file    HardDisk.c
 *  date    2009/04/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ϵ� ��ũ ��Ʈ�ѷ��� ���õ� �ҽ� ����
 */

#include "HardDisk.h"

// �ϵ� ��ũ�� �����ϴ� �ڷᱸ��
static HDDMANAGER gs_stHDDManager;

/**
 *  �ϵ� ��ũ ����̽� ����̹��� �ʱ�ȭ
 */
BOOL kInitializeHDD(void)
{
    // ���ؽ� �ʱ�ȭ
    kInitializeMutex( &( gs_stHDDManager.stMutex ) );

    // ���ͷ�Ʈ �÷��� �ʱ�ȭ
    gs_stHDDManager.bPrimaryInterruptOccur = FALSE;
    gs_stHDDManager.bSecondaryInterruptOccur = FALSE;

    // ù ��°�� �� ��° PATA ��Ʈ�� ������ ��� ��������(��Ʈ 0x3F6�� 0x376)�� 0��
    // ����Ͽ� �ϵ� ��ũ ��Ʈ�ѷ��� ���ͷ�Ʈ�� Ȱ��ȭ
    kOutPortByte( HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0 );
    kOutPortByte( HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0 );
    
    // �ϵ� ��ũ ���� ��û
    if( kReadHDDInformation( TRUE, TRUE, &( gs_stHDDManager.stHDDInformation ) ) 
            == FALSE )
    {
        gs_stHDDManager.bHDDDetected = FALSE;
        gs_stHDDManager.bCanWrite = FALSE;
        return FALSE;
    }

    // �ϵ� ��ũ�� �˻��Ǿ����� QEMU������ �� �� �ֵ��� ����
    gs_stHDDManager.bHDDDetected = TRUE;
    if( kMemCmp( gs_stHDDManager.stHDDInformation.vwModelNumber, "QEMU", 4 ) == 0 )
    {
        gs_stHDDManager.bCanWrite = TRUE;
    }
    else
    {
        gs_stHDDManager.bCanWrite = FALSE;
    }
    return TRUE;
}

/**
 *  �ϵ� ��ũ�� ���¸� ��ȯ
 */
static BYTE kReadHDDStatus( BOOL bPrimary )
{
    if( bPrimary == TRUE )
    {
        // ù ��° PATA ��Ʈ�� ���� ��������(��Ʈ 0x1F7)���� ���� ��ȯ
        return kInPortByte( HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_STATUS );
    }
    // �� ��° PATA ��Ʈ�� ���� ��������(��Ʈ 0x177)���� ���� ��ȯ
    return kInPortByte( HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_STATUS );
}

/**
 *  �ϵ� ��ũ�� Busy�� ������ ������ ���� �ð� ���� ���
 */
static BOOL kWaitForHDDNoBusy( BOOL bPrimary )
{
    QWORD qwStartTickCount;
    BYTE bStatus;

    // ��⸦ ������ �ð��� ����
    qwStartTickCount = kGetTickCount();

    // ���� �ð� ���� �ϵ� ��ũ�� Busy�� ������ ������ ���
    while( ( kGetTickCount() - qwStartTickCount ) <= HDD_WAITTIME )
    {
        // HDD�� ���¸� ��ȯ
        bStatus = kReadHDDStatus( bPrimary );

        // Busy ��Ʈ(��Ʈ 7)�� �����Ǿ� ���� ������ Busy�� ������ ���̹Ƿ� ����
        if( ( bStatus & HDD_STATUS_BUSY ) != HDD_STATUS_BUSY )
        {
            return TRUE;
        }
        kSleep( 1 );
    }
    return FALSE;
}

/**
 *  �ϵ� ��ũ�� Ready�� ������ ���� �ð� ���� ���
 */
static BOOL kWaitForHDDReady(BOOL bPrimary)
{
    QWORD qwStartTickCount;
    BYTE bStatus;

    // ��⸦ ������ �ð��� ����
    qwStartTickCount = kGetTickCount();

    // ���� �ð� ���� �ϵ� ��ũ�� Ready�� �� ������ ���
    while( ( kGetTickCount() - qwStartTickCount ) <= HDD_WAITTIME )
    {
        // HDD�� ���¸� ��ȯ
        bStatus = kReadHDDStatus( bPrimary );

        // Ready ��Ʈ(��Ʈ 6)�� �����Ǿ� ������ �����͸� ���� �غ� �� ���̹Ƿ�
        // ����
        if( ( bStatus & HDD_STATUS_READY ) == HDD_STATUS_READY )
        {
            return TRUE;
        }
        kSleep( 1 );
    }
    return FALSE;
}


/**
 *  ���ͷ�Ʈ �߻� ���θ� ����
 */
void kSetHDDInterruptFlag( BOOL bPrimary, BOOL bFlag )
{
    if( bPrimary == TRUE )
    {
        gs_stHDDManager.bPrimaryInterruptOccur = bFlag;
    }
    else
    {
        gs_stHDDManager.bSecondaryInterruptOccur = bFlag;
    }
}

/**
 *  ���ͷ�Ʈ�� �߻��� ������ ���
 */
static BOOL kWaitForHDDInterrupt( BOOL bPrimary )
{
    QWORD qwTickCount;
    
    // ��⸦ ������ �ð��� ����
    qwTickCount = kGetTickCount();
    
    // ���� �ð� ���� �ϵ� ��ũ�� ���ͷ�Ʈ�� �߻��� ������ ���
    while( kGetTickCount() - qwTickCount <= HDD_WAITTIME )
    {
        // �ϵ� ��ũ �ڷᱸ���� ���ͷ�Ʈ �߻� �÷��׸� Ȯ��
        if( ( bPrimary == TRUE ) && 
            ( gs_stHDDManager.bPrimaryInterruptOccur == TRUE ) )
        {
            return TRUE;
        }
        else if( ( bPrimary == FALSE ) && 
                 ( gs_stHDDManager.bSecondaryInterruptOccur == TRUE ) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 *  �ϵ� ��ũ�� ������ ����
 */
BOOL kReadHDDInformation( BOOL bPrimary, BOOL bMaster, HDDINFORMATION* pstHDDInformation )
{
    WORD wPortBase;
    QWORD qwLastTickCount;
    BYTE bStatus;
    BYTE bDriveFlag;
    int i;
    WORD wTemp;
    BOOL bWaitResult;

    // PATA ��Ʈ�� ���� I/O ��Ʈ�� �⺻ ��巹���� ����
    if( bPrimary == TRUE )
    {
        // ù ��° PATA ��Ʈ�̸� ��Ʈ 0x1F0�� ����
        wPortBase = HDD_PORT_PRIMARYBASE;
    }
    else
    {
        // �� ��° PATA ��Ʈ�̸� ��Ʈ 0x170�� ����
        wPortBase = HDD_PORT_SECONDARYBASE;
    }

    // ����ȭ ó��
    kLock( &(gs_stHDDManager.stMutex ) );
    
    // ���� ���� ���� Ŀ�ǵ尡 �ִٸ� ���� �ð� ���� ���� ������ ���
    if( kWaitForHDDNoBusy( bPrimary ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &(gs_stHDDManager.stMutex ) );
        return FALSE;
    }
    
    //==========================================================================
    //  LBA ��巹���� ����̺� �� ��忡 ���õ� �������� ����
    //      ����̺�� ��� ������ ������ ��
    //==========================================================================
    // ����̺�� ��� ������ ����
    if( bMaster == TRUE )
    {
        // �������̸� LBA �÷��׸� ����
        bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    }
    else
    {
        // �����̺��̸� LBA �÷��׿� �����̺� �÷��׵� ���� ����
        bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    }
    // ����̺�/��� ��������(��Ʈ 0x1F6 �Ǵ� 0x176)�� ������ ���� ����
    kOutPortByte( wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag );

    //==========================================================================
    //  Ŀ�ǵ� ���� ��, ���ͷ�Ʈ ���
    //==========================================================================
    // Ŀ�ǵ带 �޾Ƶ��� �غ� �� ������ ���� �ð� ���� ���
    if( kWaitForHDDReady( bPrimary ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &(gs_stHDDManager.stMutex ) );
        return FALSE;
    }

    // ���ͷ�Ʈ �÷��׸� �ʱ�ȭ
    kSetHDDInterruptFlag( bPrimary, FALSE );
    
    // Ŀ�ǵ� ��������(��Ʈ 0x1F7 �Ǵ� 0x177)�� ����̺� �ν� Ŀ�ǵ�(0xEC)�� ����
    kOutPortByte( wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_IDENTIFY );

    // ó���� �Ϸ�� ������ ���ͷ�Ʈ �߻��� ��ٸ�
    bWaitResult = kWaitForHDDInterrupt( bPrimary );
    // ������ �߻��ϰų� ���ͷ�Ʈ�� �߻����� ������ ������ �߻��� ���̹Ƿ� ����
    bStatus = kReadHDDStatus( bPrimary );
    if( ( bWaitResult == FALSE ) || 
        ( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR ) )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stHDDManager.stMutex ) );
        return FALSE;
    }

    //==========================================================================
    //  ������ ����
    //==========================================================================
    // �� ���͸� ����
    for( i = 0; i < 512 / 2; i++ )
    {
        ( ( WORD* ) pstHDDInformation )[ i ] =
        kInPortWord( wPortBase + HDD_PORT_INDEX_DATA );
    }

    // ���ڿ��� ����Ʈ ������ �ٽ� ��ȯ
    kSwapByteInWord( pstHDDInformation->vwModelNumber,
            sizeof( pstHDDInformation->vwModelNumber ) / 2 );
    kSwapByteInWord( pstHDDInformation->vwSerialNumber,
            sizeof( pstHDDInformation->vwSerialNumber ) / 2 );

    // ����ȭ ó��
    kUnlock( &(gs_stHDDManager.stMutex ) );
    return TRUE;
}

/**
 *  WORD ���� ����Ʈ ������ �ٲ�
 */
static void kSwapByteInWord(WORD* pwData, int iWordCount)
{
    int i;
    WORD wTemp;

    for (i = 0; i < iWordCount; i++)
    {
        wTemp = pwData[ i ];
        pwData[ i ] = (wTemp >> 8 ) | (wTemp << 8 );
    }
}

/**
 *  �ϵ� ��ũ�� ���͸� ����
 *      �ִ� 256���� ���͸� ���� �� ����
 *      ������ ���� ���� ���� ��ȯ
 */
int kReadHDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
        char* pcBuffer )
{
    WORD wPortBase;
    int i, j;
    BYTE bDriveFlag;
    BYTE bStatus;
    long lReadCount = 0;
    BOOL bWaitResult;

    // ���� �˻�. �ִ� 256 ���͸� ó���� �� ����
    if( ( gs_stHDDManager.bHDDDetected == FALSE ) ||
        ( iSectorCount <= 0 ) || ( 256 < iSectorCount ) ||
        ( ( dwLBA + iSectorCount ) >= gs_stHDDManager.stHDDInformation.dwTotalSectors ) )
    {
        return 0;
    }

    // PATA ��Ʈ�� ���� I/O ��Ʈ�� �⺻ ��巹���� ����
    if( bPrimary == TRUE )
    {
        // ù ��° PATA ��Ʈ�̸� ��Ʈ 0x1F0�� ����
        wPortBase = HDD_PORT_PRIMARYBASE;
    }
    else
    {
        // �� ��° PATA ��Ʈ�̸� ��Ʈ 0x170�� ����
        wPortBase = HDD_PORT_SECONDARYBASE;
    }

    // ����ȭ ó��
    kLock( &( gs_stHDDManager.stMutex ) );
    
    // ���� ���� ���� Ŀ�ǵ尡 �ִٸ� ���� �ð� ���� ���� ������ ���
    if( kWaitForHDDNoBusy( bPrimary ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stHDDManager.stMutex ) );
        return FALSE;
    }

    //==========================================================================
    //  ������ �������� ����
    //      LBA ����� ���, ���� ��ȣ -> �Ǹ��� ��ȣ -> ��� ��ȣ�� ������ 
    //      LBA ��巹���� ����
    //==========================================================================
    // ���� �� ��������(��Ʈ 0x1F2 �Ǵ� 0x172)�� ���� ���� ���� ����
    kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount );
    // ���� ��ȣ ��������(��Ʈ 0x1F3 �Ǵ� 0x173)�� ���� ���� ��ġ(LBA 0~7��Ʈ)�� ����
    kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA );
    // �Ǹ��� LSB ��������(��Ʈ 0x1F4 �Ǵ� 0x174)�� ���� ���� ��ġ(LBA 8~15��Ʈ)�� ����
    kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8 );
    // �Ǹ��� MSB ��������(��Ʈ 0x1F5 �Ǵ� 0x175)�� ���� ���� ��ġ(LBA 16~23��Ʈ)�� ����
    kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16 );
    // ����̺�� ��� ������ ����
    if( bMaster == TRUE )
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    }
    else
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    }
    // ����̺�/��� ��������(��Ʈ 0x1F6 �Ǵ� 0x176)�� ���� ������ ��ġ(LBA 24~27��Ʈ)��
    // ������ ���� ���� ����
    kOutPortByte( wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | ( (dwLBA
            >> 24 ) & 0x0F ) );

    //==========================================================================
    //  Ŀ�ǵ� ����
    //==========================================================================
    // Ŀ�ǵ带 �޾Ƶ��� �غ� �� ������ ���� �ð� ���� ���
    if( kWaitForHDDReady( bPrimary ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stHDDManager.stMutex ) );
        return FALSE;
    }

    // ���ͷ�Ʈ �÷��׸� �ʱ�ȭ
    kSetHDDInterruptFlag( bPrimary, FALSE );
    
    // Ŀ�ǵ� ��������(��Ʈ 0x1F7 �Ǵ� 0x177)�� �б�(0x20)�� ����
    kOutPortByte( wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_READ );

    //==========================================================================
    //  ���ͷ�Ʈ ��� ��, ������ ����
    //==========================================================================
    // ���� ����ŭ ������ ���鼭 ������ ����
    for( i = 0 ; i < iSectorCount ; i++ )
    {
        // ������ �߻��ϸ� ����
        bStatus = kReadHDDStatus( bPrimary );
        if( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
        {
            kPrintf( "Error Occur\n" );
            // ����ȭ ó��
            kUnlock( &( gs_stHDDManager.stMutex ) );
            return i;
        }

        // DATAREQUEST ��Ʈ�� �������� �ʾ����� �����Ͱ� ���ŵǱ� ��ٸ�
        if( ( bStatus & HDD_STATUS_DATAREQUEST ) != HDD_STATUS_DATAREQUEST )
        {
            // ó���� �Ϸ�� ������ ���� �ð� ���� ���ͷ�Ʈ�� ��ٸ�
            bWaitResult = kWaitForHDDInterrupt( bPrimary );
            kSetHDDInterruptFlag( bPrimary, FALSE );
            // ���ͷ�Ʈ�� �߻����� ������ ������ �߻��� ���̹Ƿ� ����
            if( bWaitResult == FALSE )
            {
                kPrintf( "Interrupt Not Occur\n" );
                // ����ȭ ó��
                kUnlock( &( gs_stHDDManager.stMutex ) );
                return FALSE;
            }
        }
        
        // �� ���͸� ����
        for( j = 0 ; j < 512 / 2 ; j++ )
        {
            ( ( WORD* ) pcBuffer )[ lReadCount++ ]
                    = kInPortWord( wPortBase + HDD_PORT_INDEX_DATA );
        }
    }

    // ����ȭ ó��
    kUnlock( &( gs_stHDDManager.stMutex ) );
    return i;
}

int kWriteHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
        char* pcBuffer)
{
    WORD wPortBase;
    WORD wTemp;
    int i, j;
    BYTE bDriveFlag;
    BYTE bStatus;
    long lReadCount = 0;
    BOOL bWaitResult;

    if( ( gs_stHDDManager.bCanWrite == FALSE ) || 
        ( iSectorCount <= 0 ) || ( 256 < iSectorCount ) ||
        ( ( dwLBA + iSectorCount ) >= gs_stHDDManager.stHDDInformation.dwTotalSectors ) )

    {
        return 0;
    }

    if( bPrimary == TRUE )
    {
        wPortBase = HDD_PORT_PRIMARYBASE;
    }
    else
    {
        wPortBase = HDD_PORT_SECONDARYBASE;
    }

    if( kWaitForHDDNoBusy( bPrimary ) == FALSE )
    {
        return FALSE;
    }

    kLock( &(gs_stHDDManager.stMutex ) );

    kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount );
    kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA );
    kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8 );
    kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16 );
    if( bMaster == TRUE )
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    }
    else
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    }
    kOutPortByte( wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | ( (dwLBA
            >> 24 ) & 0x0F ) );

    if( kWaitForHDDReady( bPrimary ) == FALSE )
    {
        kUnlock( &( gs_stHDDManager.stMutex ) );
        return FALSE;
    }

    kOutPortByte( wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_WRITE );
    
    while( 1 )
    {
        bStatus = kReadHDDStatus( bPrimary );
        if( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
        {
            kUnlock( &(gs_stHDDManager.stMutex ) );
            return 0;
        }
        
        if( ( bStatus & HDD_STATUS_DATAREQUEST ) == HDD_STATUS_DATAREQUEST )
        {
            break;
        }
        
        kSleep( 1 );        
    }

    for( i = 0 ; i < iSectorCount ; i++)
    {
        kSetHDDInterruptFlag( bPrimary, FALSE );
        for( j = 0 ; j < 512 / 2 ; j++ )
        {
            kOutPortWord( wPortBase + HDD_PORT_INDEX_DATA, 
                         ( ( WORD* ) pcBuffer )[ lReadCount++ ]);
        }
        
        bStatus = kReadHDDStatus( bPrimary );
        if( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
        {
            kUnlock( &(gs_stHDDManager.stMutex ) );
            return i;
        }
        
        if( ( bStatus & HDD_STATUS_DATAREQUEST ) != HDD_STATUS_DATAREQUEST )
        {
            bWaitResult = kWaitForHDDInterrupt( bPrimary );
            kSetHDDInterruptFlag( bPrimary, FALSE );
            if( bWaitResult == FALSE )
            {
                kUnlock( &( gs_stHDDManager.stMutex ) );
                return FALSE;
            }
        }        
    }
    
    kUnlock( &(gs_stHDDManager.stMutex ) );
    return i;
=======
#include "HardDisk.h"

static HDDMANAGER gs_stHDDManager;

BOOL kInitializeHDD(void)
{
    kInitializeMutex( &( gs_stHDDManager.stMutex ) );

    gs_stHDDManager.bPrimaryInterruptOccur = FALSE;
    gs_stHDDManager.bSecondaryInterruptOccur = FALSE;

    kOutPortByte( HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0 );
    kOutPortByte( HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0 );
    
    if( kReadHDDInformation( TRUE, TRUE, &( gs_stHDDManager.stHDDInformation ) ) 
            == FALSE )
    {
        gs_stHDDManager.bHDDDetected = FALSE;
        gs_stHDDManager.bCanWrite = FALSE;
        return FALSE;
    }
    gs_stHDDManager.bHDDDetected = TRUE;
    if( kMemCmp( gs_stHDDManager.stHDDInformation.vwModelNumber, "QEMU", 4 ) == 0 )
    {
        gs_stHDDManager.bCanWrite = TRUE;
    }
    else
    {
        gs_stHDDManager.bCanWrite = FALSE;
    }
    return TRUE;
}

static BYTE kReadHDDStatus( BOOL bPrimary )
{
    if( bPrimary == TRUE )
    {
        return kInPortByte( HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_STATUS );
    }
    return kInPortByte( HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_STATUS );
}

static BOOL kWaitForHDDNoBusy( BOOL bPrimary )
{
    QWORD qwStartTickCount;
    BYTE bStatus;

    qwStartTickCount = kGetTickCount();

    while( ( kGetTickCount() - qwStartTickCount ) <= HDD_WAITTIME )
    {
        bStatus = kReadHDDStatus( bPrimary );

        if( ( bStatus & HDD_STATUS_BUSY ) != HDD_STATUS_BUSY )
        {
            return TRUE;
        }
        kSleep( 1 );
    }
    return FALSE;
}

static BOOL kWaitForHDDReady(BOOL bPrimary)
{
    QWORD qwStartTickCount;
    BYTE bStatus;

    qwStartTickCount = kGetTickCount();

    while( ( kGetTickCount() - qwStartTickCount ) <= HDD_WAITTIME )
    {
        bStatus = kReadHDDStatus( bPrimary );
        if( ( bStatus & HDD_STATUS_READY ) == HDD_STATUS_READY )
        {
            return TRUE;
        }
        kSleep( 1 );
    }
    return FALSE;
}

void kSetHDDInterruptFlag( BOOL bPrimary, BOOL bFlag )
{
    if( bPrimary == TRUE )
    {
        gs_stHDDManager.bPrimaryInterruptOccur = bFlag;
    }
    else
    {
        gs_stHDDManager.bSecondaryInterruptOccur = bFlag;
    }
}

static BOOL kWaitForHDDInterrupt( BOOL bPrimary )
{
    QWORD qwTickCount;
    
    qwTickCount = kGetTickCount();
    
    while( kGetTickCount() - qwTickCount <= HDD_WAITTIME )
    {
        if( ( bPrimary == TRUE ) && 
            ( gs_stHDDManager.bPrimaryInterruptOccur == TRUE ) )
        {
            return TRUE;
        }
        else if( ( bPrimary == FALSE ) && 
                 ( gs_stHDDManager.bSecondaryInterruptOccur == TRUE ) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL kReadHDDInformation( BOOL bPrimary, BOOL bMaster, HDDINFORMATION* pstHDDInformation )
{
    WORD wPortBase;
    QWORD qwLastTickCount;
    BYTE bStatus;
    BYTE bDriveFlag;
    int i;
    WORD wTemp;
    BOOL bWaitResult;

    if( bPrimary == TRUE )
    {
        wPortBase = HDD_PORT_PRIMARYBASE;
    }
    else
    {
        wPortBase = HDD_PORT_SECONDARYBASE;
    }

    kLock( &(gs_stHDDManager.stMutex ) );
    if( kWaitForHDDNoBusy( bPrimary ) == FALSE )
    {
        kUnlock( &(gs_stHDDManager.stMutex ) );
        return FALSE;
    }
    
    if( bMaster == TRUE )
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    }
    else
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    }
    kOutPortByte( wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag );

    if( kWaitForHDDReady( bPrimary ) == FALSE )
    {
        kUnlock( &(gs_stHDDManager.stMutex ) );
        return FALSE;
    }

    kSetHDDInterruptFlag( bPrimary, FALSE );
    
    kOutPortByte( wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_IDENTIFY );

    bWaitResult = kWaitForHDDInterrupt( bPrimary );
    bStatus = kReadHDDStatus( bPrimary );
    if( ( bWaitResult == FALSE ) || 
        ( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR ) )
    {
        kUnlock( &( gs_stHDDManager.stMutex ) );
        return FALSE;
    }

    for( i = 0; i < 512 / 2; i++ )
    {
        ( ( WORD* ) pstHDDInformation )[ i ] =
        kInPortWord( wPortBase + HDD_PORT_INDEX_DATA );
    }
    kSwapByteInWord( pstHDDInformation->vwModelNumber,
            sizeof( pstHDDInformation->vwModelNumber ) / 2 );
    kSwapByteInWord( pstHDDInformation->vwSerialNumber,
            sizeof( pstHDDInformation->vwSerialNumber ) / 2 );

    kUnlock( &(gs_stHDDManager.stMutex ) );
    return TRUE;
}

static void kSwapByteInWord(WORD* pwData, int iWordCount)
{
    int i;
    WORD wTemp;

    for (i = 0; i < iWordCount; i++)
    {
        wTemp = pwData[ i ];
        pwData[ i ] = (wTemp >> 8 ) | (wTemp << 8 );
    }
}

int kReadHDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
        char* pcBuffer )
{
    WORD wPortBase;
    int i, j;
    BYTE bDriveFlag;
    BYTE bStatus;
    long lReadCount = 0;
    BOOL bWaitResult;

    if( ( gs_stHDDManager.bHDDDetected == FALSE ) ||
        ( iSectorCount <= 0 ) || ( 256 < iSectorCount ) ||
        ( ( dwLBA + iSectorCount ) >= gs_stHDDManager.stHDDInformation.dwTotalSectors ) )
    {
        return 0;
    }

    if( bPrimary == TRUE )
    {
        wPortBase = HDD_PORT_PRIMARYBASE;
    }
    else
    {
        wPortBase = HDD_PORT_SECONDARYBASE;
    }

    kLock( &( gs_stHDDManager.stMutex ) );
    if( kWaitForHDDNoBusy( bPrimary ) == FALSE )
    {
        kUnlock( &( gs_stHDDManager.stMutex ) );
        return FALSE;
    }

    kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount );
    kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA );
    kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8 );
    kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16 );
    if( bMaster == TRUE )
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    }
    else
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    }
    kOutPortByte( wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | ( (dwLBA
            >> 24 ) & 0x0F ) );

    if( kWaitForHDDReady( bPrimary ) == FALSE )
    {
        kUnlock( &( gs_stHDDManager.stMutex ) );
        return FALSE;
    }
    kSetHDDInterruptFlag( bPrimary, FALSE );
    
    kOutPortByte( wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_READ );

    for( i = 0 ; i < iSectorCount ; i++ )
    {
        bStatus = kReadHDDStatus( bPrimary );
        if( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
        {
            kPrintf( "Error Occur\n" );
            kUnlock( &( gs_stHDDManager.stMutex ) );
            return i;
        }

        if( ( bStatus & HDD_STATUS_DATAREQUEST ) != HDD_STATUS_DATAREQUEST )
        {
            bWaitResult = kWaitForHDDInterrupt( bPrimary );
            kSetHDDInterruptFlag( bPrimary, FALSE );
            if( bWaitResult == FALSE )
            {
                kPrintf( "Interrupt Not Occur\n" );
                kUnlock( &( gs_stHDDManager.stMutex ) );
                return FALSE;
            }
        }
        
        for( j = 0 ; j < 512 / 2 ; j++ )
        {
            ( ( WORD* ) pcBuffer )[ lReadCount++ ]
                    = kInPortWord( wPortBase + HDD_PORT_INDEX_DATA );
        }
    }

    kUnlock( &( gs_stHDDManager.stMutex ) );
    return i;
}

int kWriteHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
        char* pcBuffer)
{
    WORD wPortBase;
    WORD wTemp;
    int i, j;
    BYTE bDriveFlag;
    BYTE bStatus;
    long lReadCount = 0;
    BOOL bWaitResult;

    if( ( gs_stHDDManager.bCanWrite == FALSE ) || 
        ( iSectorCount <= 0 ) || ( 256 < iSectorCount ) ||
        ( ( dwLBA + iSectorCount ) >= gs_stHDDManager.stHDDInformation.dwTotalSectors ) )

    {
        return 0;
    }

    if( bPrimary == TRUE )
    {
        wPortBase = HDD_PORT_PRIMARYBASE;
    }
    else
    {
        wPortBase = HDD_PORT_SECONDARYBASE;
    }
    if( kWaitForHDDNoBusy( bPrimary ) == FALSE )
    {
        return FALSE;
    }
    kLock( &(gs_stHDDManager.stMutex ) );

    kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount );
    kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA );
    kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8 );
    kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16 );
    
    if( bMaster == TRUE )
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    }
    else
    {
        bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    }
    kOutPortByte( wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | ( (dwLBA
            >> 24 ) & 0x0F ) );

    if( kWaitForHDDReady( bPrimary ) == FALSE )
    {
        kUnlock( &( gs_stHDDManager.stMutex ) );
        return FALSE;
    }

    kOutPortByte( wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_WRITE );
    
    while( 1 )
    {
        bStatus = kReadHDDStatus( bPrimary );
        
        if( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
        {
            kUnlock( &(gs_stHDDManager.stMutex ) );
            return 0;
        }
        
        if( ( bStatus & HDD_STATUS_DATAREQUEST ) == HDD_STATUS_DATAREQUEST )
        {
            break;
        }
        
        kSleep( 1 );        
    }

    for( i = 0 ; i < iSectorCount ; i++)
    {
        kSetHDDInterruptFlag( bPrimary, FALSE );
        for( j = 0 ; j < 512 / 2 ; j++ )
        {
            kOutPortWord( wPortBase + HDD_PORT_INDEX_DATA, 
                         ( ( WORD* ) pcBuffer )[ lReadCount++ ]);
        }
        
        bStatus = kReadHDDStatus( bPrimary );
        if( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
        {
            kUnlock( &(gs_stHDDManager.stMutex ) );
            return i;
        }
        if( ( bStatus & HDD_STATUS_DATAREQUEST ) != HDD_STATUS_DATAREQUEST )
        {
            
            bWaitResult = kWaitForHDDInterrupt( bPrimary );
            kSetHDDInterruptFlag( bPrimary, FALSE );
            
            if( bWaitResult == FALSE )
            {
                kUnlock( &( gs_stHDDManager.stMutex ) );
                return FALSE;
            }
        }        
    }
    
    kUnlock( &(gs_stHDDManager.stMutex ) );
    return i;
>>>>>>> NEC
}