<<<<<<< HEAD
/**
 *  file    DynmaicMemory.h
 *  date    2009/04/11
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���� �޸� �Ҵ�� ������ ���õ� ��� ����
 */

=======
>>>>>>> NEC
#ifndef __DYNAMICMEMORY_H__
#define __DYNAMICMEMORY_H__

#include "Types.h"

<<<<<<< HEAD
////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���� �޸� ������ ���� ��巹��, 1Mbyte ������ ����
#define DYNAMICMEMORY_START_ADDRESS     ( ( TASK_STACKPOOLADDRESS + \
        ( TASK_STACKSIZE * TASK_MAXCOUNT ) + 0xfffff ) & 0xfffffffffff00000 )
// ���� ����� �ּ� ũ��, 1KB
#define DYNAMICMEMORY_MIN_SIZE          ( 1 * 1024 )

// ��Ʈ���� �÷���
#define DYNAMICMEMORY_EXIST             0x01
#define DYNAMICMEMORY_EMPTY             0x00

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// ��Ʈ���� �����ϴ� �ڷᱸ��
=======
#define DYNAMICMEMORY_START_ADDRESS     ( ( TASK_STACKPOOLADDRESS + \
        ( TASK_STACKSIZE * TASK_MAXCOUNT ) + 0xfffff ) & 0xfffffffffff00000 )
#define DYNAMICMEMORY_MIN_SIZE          ( 1 * 1024 )
#define DYNAMICMEMORY_EXIST             0x01
#define DYNAMICMEMORY_EMPTY             0x00

>>>>>>> NEC
typedef struct kBitmapStruct
{
    BYTE* pbBitmap;
    QWORD qwExistBitCount;
} BITMAP;

<<<<<<< HEAD
// ���� ����� �����ϴ� �ڷᱸ��
typedef struct kDynamicMemoryManagerStruct
{
    // ��� ����Ʈ�� �� ������ ���� ũ�Ⱑ ���� ���� ����� ����, �׸��� �Ҵ�� �޸� ũ��
=======
typedef struct kDynamicMemoryManagerStruct
{
>>>>>>> NEC
    int iMaxLevelCount;
    int iBlockCountOfSmallestBlock;
    QWORD qwUsedSize;
    
<<<<<<< HEAD
    // ��� Ǯ�� ���� ��巹���� ������ ��巹��
    QWORD qwStartAddress;
    QWORD qwEndAddress;
    
    // �Ҵ�� �޸𸮰� ���� ��� ����Ʈ�� �ε����� �����ϴ� ������ ��Ʈ�� �ڷᱸ���� 
    // ��巹��
=======
    QWORD qwStartAddress;
    QWORD qwEndAddress;
    
>>>>>>> NEC
    BYTE* pbAllocatedBlockListIndex;
    BITMAP* pstBitmapOfLevel;
} DYNAMICMEMORY;

<<<<<<< HEAD
////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
=======
>>>>>>> NEC
void kInitializeDynamicMemory( void );
void* kAllocateMemory( QWORD qwSize );
BOOL kFreeMemory( void* pvAddress );
void kGetDynamicMemoryInformation( QWORD* pqwDynamicMemoryStartAddress, 
        QWORD* pqwDynamicMemoryTotalSize, QWORD* pqwMetaDataSize, 
        QWORD* pqwUsedMemorySize ); 
DYNAMICMEMORY* kGetDynamicMemoryManager( void );

static QWORD kCalculateDynamicMemorySize( void );
static int kCalculateMetaBlockCount( QWORD qwDynamicRAMSize );
static int kAllocationBuddyBlock( QWORD qwAlignedSize );
static QWORD kGetBuddyBlockSize( QWORD qwSize );
static int kGetBlockListIndexOfMatchSize( QWORD qwAlignedSize );
static int kFindFreeBlockInBitmap( int iBlockListIndex );
static void kSetFlagInBitmap( int iBlockListIndex, int iOffset, BYTE bFlag );
static BOOL kFreeBuddyBlock( int iBlockListIndex, int iBlockOffset );
static BYTE kGetFlagInBitmap( int iBlockListIndex, int iOffset );

<<<<<<< HEAD
#endif /*__DYNAMICMEMORY_H__*/
=======
#endif
>>>>>>> NEC
