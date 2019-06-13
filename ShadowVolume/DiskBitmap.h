#ifndef _DP_BITMAP_H_
#define _DP_BITMAP_H_

#define BITMAP_ERR_OUTOFRANGE	-1
#define BITMAP_ERR_ALLOCMEMORY	-2
#define BITMAP_SUCCESS			0
#define BITMAP_BIT_SET			1
#define BITMAP_BIT_CLEAR		2
#define BITMAP_BIT_UNKNOW		3
#define BITMAP_RANGE_SET		4
#define BITMAP_RANGE_CLEAR		5
#define BITMAP_RANGE_BLEND		6
#define BITMAP_RANGE_SIZE		25600
#define BITMAP_RANGE_SIZE_SMALL 256
#define BITMAP_RANGE_SIZE_MAX	51684
#define BITMAP_RANGE_AMOUNT		16*1024

typedef UCHAR tBitmap;

#include <pshpack1.h>

typedef struct _DP_BITMAP_
{
    //������е�ÿ�������ж����ֽڣ���ͬ��Ҳ˵����bitmap��һ��λ����Ӧ���ֽ���
    ULONG       sectorSize;
    //ÿ��byte�����м���bit��һ���������8
    ULONG       byteSize;
    //ÿ�����Ƕ��byte��
    ULONG       regionSize;
    //���bitmap�ܹ��ж��ٸ���
    ULONG       regionNumber;
    //������Ӧ�˶��ٸ�ʵ�ʵ�byte���������Ӧ����sectorSize*byteSize*regionSize
    ULONG       regionReferSize;
    //���bitmap��Ӧ�˶��ٸ�ʵ�ʵ�byte���������Ӧ����sectorSize*byteSize*regionSize*regionNumber
    ULONGLONG   bitmapReferSize;
    //ָ��bitmap�洢�ռ��ָ��
    tBitmap**   Bitmap;
    //���ڴ�ȡbitmap����
    void*       lockBitmap;
} DP_BITMAP, *PDP_BITMAP;

#include <poppack.h>
                    
NTSTATUS 
DPBitmapInit(
    DP_BITMAP **	sbitmap,
    ULONG           sectorSize,
    ULONG           byteSize,
    ULONG           regionSize,
    ULONG           regionNumber
);
                    
void
DPBitmapFree(
    DP_BITMAP* bitmap
);
                    
NTSTATUS 
DPBitmapSet(
    DP_BITMAP *		bitmap,
    ULONGLONG       offset,
    ULONG           length
);
                    
NTSTATUS 
DPBitmapGet(
    DP_BITMAP *     bitmap,
    ULONGLONG       offset,
    ULONG           length,
    void *          bufInOut,
    void *          bufIn
);
                    
NTSTATUS
DPBitmapTestRange(
    DP_BITMAP *     bitmap,
    ULONGLONG       offset,
    ULONG           length
);

BOOLEAN
DPBitmapTestBit(
    DP_BITMAP *     bitmap,
    ULONGLONG       offset
);

ULONGLONG
DPBitmapGetNextOffset(
    DP_BITMAP *     bitmap,
    ULONGLONG       start,
    BOOLEAN         usage
);

#endif
