#include <ntifs.h>
#include <windef.h>
#include "DiskBitmap.h"

static tBitmap bitmapMask[8] =
{
    //��Ҫ�õ���bitmap��λ����
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

void * 
DPBitmapAlloc(
    int             poolType,
    ULONG           length
)
{
    //���ݲ��������䲻ͬ���͵��ڴ棬����ȫ��������ǷǷ�ҳ�����ڴ�
    if (0 == poolType)
    {
        return ExAllocatePoolWithTag(NonPagedPool, length, 'mbpD');
    }
    else if (1 == poolType)
    {
        return ExAllocatePoolWithTag(PagedPool, length, 'mbpD');
    }
    else
    {
        return NULL;
    }
}

void 
DPBitmapFree(
    DP_BITMAP* bitmap
)
{
    //�ͷ�bitmap
    DWORD i = 0;

    if (NULL != bitmap)
    {
        if (NULL != bitmap->Bitmap)
        {
            for (i = 0; i < bitmap->regionNumber; i++)
            {
                if (NULL != *(bitmap->Bitmap + i))
                {
                    //����ײ�Ŀ鿪ʼ�ͷţ����п鶼��ѯһ��				
                    ExFreePool(*(bitmap->Bitmap + i));
                }
            }
            //�ͷſ��ָ��
            ExFreePool(bitmap->Bitmap);
        }
        //�ͷ�bitmap����
        ExFreePool(bitmap);
    }
}

NTSTATUS 
DPBitmapInit(
    DP_BITMAP **    bitmap,
    ULONG           sectorSize,
    ULONG           byteSize,
    ULONG           regionSize,
    ULONG           regionNumber
)
{
    DP_BITMAP * myBitmap = NULL;
    NTSTATUS status = STATUS_SUCCESS;

    //������������ʹ���˴���Ĳ������·��������ȴ���
    if (NULL == bitmap || 0 == sectorSize ||
        0 == byteSize || 0 == regionSize || 0 == regionNumber)
    {
        return STATUS_UNSUCCESSFUL;
    }
    __try
    {
        //����һ��bitmap�ṹ������������ζ�Ҫ����ģ�����ṹ�൱��һ��bitmap��handle	
        if (NULL == (myBitmap = (DP_BITMAP*)DPBitmapAlloc(0, sizeof(DP_BITMAP))))
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }
        //��սṹ
        memset(myBitmap, 0, sizeof(DP_BITMAP));
        //���ݲ����Խṹ�еĳ�Ա���и�ֵ
        myBitmap->sectorSize = sectorSize;
        myBitmap->byteSize = byteSize;
        myBitmap->regionSize = regionSize;
        myBitmap->regionNumber = regionNumber;
        myBitmap->regionReferSize = sectorSize * byteSize * regionSize;
        myBitmap->bitmapReferSize = (__int64)sectorSize * (__int64)byteSize * (__int64)regionSize * (__int64)regionNumber;
        //�����regionNumber��ô���ָ��region��ָ�룬����һ��ָ������
        if (NULL == (myBitmap->Bitmap = (tBitmap **)DPBitmapAlloc(0, sizeof(tBitmap*) * regionNumber)))
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }
        //���ָ������
        memset(myBitmap->Bitmap, 0, sizeof(tBitmap*) * regionNumber);
        *bitmap = myBitmap;
        status = STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_UNSUCCESSFUL;
    }
    if (!NT_SUCCESS(status))
    {
        if (NULL != myBitmap)
        {
            DPBitmapFree(myBitmap);
        }
        *bitmap = NULL;
    }
    return status;
}

NTSTATUS
DPBitmapSet(
    DP_BITMAP *     bitmap,
    ULONGLONG       offset,
    ULONG           length
)
{
    ULONGLONG i = 0;
    ULONG myRegion = 0, myRegionEnd = 0;
    ULONG myRegionOffset = 0, myRegionOffsetEnd = 0;
    ULONG myByteOffset = 0, myByteOffsetEnd = 0;
    ULONG myBitPos = 0;
    NTSTATUS status = STATUS_SUCCESS;
    ULONGLONG setBegin = 0, setEnd = 0;

    __try
    {
        //������
        if (NULL == bitmap)
        {
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }
        if (0 != offset % bitmap->sectorSize || 0 != length % bitmap
            ->sectorSize)
        {
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }

        //����Ҫ���õ�ƫ�����ͳ�����������Ҫʹ�õ���Щregion�������Ҫ�Ļ����ͷ�������ָ����ڴ�ռ�
        myRegion = (ULONG)(offset / (ULONGLONG)bitmap->regionReferSize);
        myRegionEnd = (ULONG)((offset + (ULONGLONG)length) / (ULONGLONG)bitmap->regionReferSize);
        for (i = myRegion; i <= myRegionEnd; ++i)
        {
            if (NULL == *(bitmap->Bitmap + i))
            {
                if (NULL == (*(bitmap->Bitmap + i) = (tBitmap*)DPBitmapAlloc(0, sizeof(tBitmap) * bitmap->regionSize)))
                {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    __leave;
                }
                else
                {
                    memset(*(bitmap->Bitmap + i), 0, sizeof(tBitmap) * bitmap->regionSize);
                }
            }
        }

        //��ʼ����bitmap������������Ҫ��Ҫ���õ�������byte���룬�������԰�byte���ö�����Ҫ��bit���ã��ӿ������ٶ�
        //����û��byte������������ֹ����õ�����
        for (i = offset; i < offset + (ULONGLONG)length; i += bitmap->sectorSize)
        {
            myRegion = (ULONG)(i / (ULONGLONG)bitmap->regionReferSize);
            myRegionOffset = (ULONG)(i % (ULONGLONG)bitmap->regionReferSize);
            myByteOffset = myRegionOffset / bitmap->byteSize / bitmap->sectorSize;
            myBitPos = (myRegionOffset / bitmap->sectorSize) % bitmap->byteSize;
            if (0 == myBitPos)
            {
                setBegin = i;
                break;
            }
            *(*(bitmap->Bitmap + myRegion) + myByteOffset) |= bitmapMask[myBitPos];
        }
        if (i >= offset + (ULONGLONG)length)
        {
            status = STATUS_SUCCESS;
            __leave;
        }

        for (i = offset + (ULONGLONG)length - bitmap->sectorSize; i >= offset; i -= bitmap->sectorSize)
        {
            myRegion = (ULONG)(i / (ULONGLONG)bitmap->regionReferSize);
            myRegionOffset = (ULONG)(i % (ULONGLONG)bitmap->regionReferSize);
            myByteOffset = myRegionOffset / bitmap->byteSize / bitmap->sectorSize;
            myBitPos = (myRegionOffset / bitmap->sectorSize) % bitmap->byteSize;
            if (7 == myBitPos)
            {
                setEnd = i;
                break;
            }
            *(*(bitmap->Bitmap + myRegion) + myByteOffset) |= bitmapMask[myBitPos];
        }

        if (i < offset || setEnd == setBegin)
        {
            status = STATUS_SUCCESS;
            __leave;
        }

        myRegionEnd = (ULONG)(setEnd / (ULONGLONG)bitmap->regionReferSize);

        for (i = setBegin; i <= setEnd;)
        {
            myRegion = (ULONG)(i / (ULONGLONG)bitmap->regionReferSize);
            myRegionOffset = (ULONG)(i % (ULONGLONG)bitmap->regionReferSize);
            myByteOffset = myRegionOffset / bitmap->byteSize / bitmap->sectorSize;
            //����������õ�����û�п�����region��ֻ��Ҫʹ��memsetȥ����byte������Ȼ����������
            if (myRegion == myRegionEnd)
            {
                myRegionOffsetEnd = (ULONG)(setEnd % (__int64)bitmap->regionReferSize);
                myByteOffsetEnd = myRegionOffsetEnd / bitmap->byteSize / bitmap->sectorSize;
                memset(*(bitmap->Bitmap + myRegion) + myByteOffset, 0xff, myByteOffsetEnd - myByteOffset + 1);
                break;
            }
            //����������õ������������region����Ҫ����������
            else
            {
                myRegionOffsetEnd = bitmap->regionReferSize;
                myByteOffsetEnd = myRegionOffsetEnd / bitmap->byteSize / bitmap->sectorSize;
                memset(*(bitmap->Bitmap + myRegion) + myByteOffset, 0xff, myByteOffsetEnd - myByteOffset);
                i += (myByteOffsetEnd - myByteOffset) * bitmap->byteSize * bitmap->sectorSize;
            }
        }
        status = STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_UNSUCCESSFUL;
    }

    if (!NT_SUCCESS(status))
    {

    }
    return status;
}

NTSTATUS
DPBitmapGet(
    DP_BITMAP *     bitmap,
    ULONGLONG       offset,
    ULONG           length,
    void *          bufInOut,
    void *          bufIn
)
{
    ULONG i = 0;
    ULONG myRegion = 0;
    ULONG myRegionOffset = 0;
    ULONG myByteOffset = 0;
    ULONG myBitPos = 0;
    NTSTATUS status = STATUS_SUCCESS;

    __try
    {
        //������
        if (NULL == bitmap || NULL == bufInOut || NULL == bufIn)
        {
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }
        if (0 != offset % bitmap->sectorSize || 0 != length % bitmap->sectorSize)
        {
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }

        //������Ҫ��ȡ��λͼ��Χ�����������λ������Ϊ1������Ҫ��bufIn������ָ�����Ӧλ�õ����ݿ�����bufInOut��
        for (i = 0; i < length; i += bitmap->sectorSize)
        {
            myRegion = (ULONG)((offset + (ULONGLONG)i) / (ULONGLONG)bitmap->regionReferSize);

            myRegionOffset = (ULONG)((offset + (ULONGLONG)i) % (ULONGLONG)bitmap->regionReferSize);

            myByteOffset = myRegionOffset / bitmap->byteSize / bitmap->sectorSize;

            myBitPos = (myRegionOffset / bitmap->sectorSize) % bitmap->byteSize;

            if (NULL != *(bitmap->Bitmap + myRegion) && (*(*(bitmap->Bitmap + myRegion) + myByteOffset) &bitmapMask[myBitPos]))
            {
                memcpy((tBitmap*)bufInOut + i, (tBitmap*)bufIn + i, bitmap->sectorSize);
            }
        }

        status = STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}

NTSTATUS DPBitmapTestRange(
    DP_BITMAP *     bitmap,
    ULONGLONG       offset,
    ULONG           length
)
{
    char flag = 0;
    ULONG i = 0;
    ULONG myRegion = 0;
    ULONG myRegionOffset = 0;
    ULONG myByteOffset = 0;
    ULONG myBitPos = 0;
    NTSTATUS ret = BITMAP_BIT_UNKNOW;

    __try
    {
        //������
        if (NULL == bitmap || offset + length > bitmap->bitmapReferSize)
        {
            ret = BITMAP_BIT_UNKNOW;

            __leave;
        }

        for (i = 0; i < length; i += bitmap->sectorSize)
        {
            //�����Ҫ���Ե�bitmap��Χ���в��ԣ����ȫ��Ϊ0�򷵻�BITMAP_RANGE_CLEAR�����ȫ��Ϊ1���򷵻�BITMAP_RANGE_SET�����Ϊ0��1����򷵻�BITMAP_RANGE_BLEND
            myRegion = (ULONG)((offset + (ULONGLONG)i) / (ULONGLONG)bitmap->regionReferSize);

            myRegionOffset = (ULONG)((offset + (ULONGLONG)i) % (ULONGLONG)bitmap->regionReferSize);

            myByteOffset = myRegionOffset / bitmap->byteSize / bitmap->sectorSize;

            myBitPos = (myRegionOffset / bitmap->sectorSize) % bitmap->byteSize;

            if (NULL != *(bitmap->Bitmap + myRegion) && (*(*(bitmap->Bitmap + myRegion) + myByteOffset) &bitmapMask[myBitPos]))
            {
                flag |= 0x2;
            }
            else
            {
                flag |= 0x1;
            }

            if (flag == 0x3)
            {
                break;
            }
        }

        if (0x2 == flag)
        {
            ret = BITMAP_RANGE_SET;
        }
        else if (0x01 == flag)
        {
            ret = BITMAP_RANGE_CLEAR;
        }
        else if (0x03 == flag)
        {
            ret = BITMAP_RANGE_BLEND;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        ret = BITMAP_BIT_UNKNOW;
    }

    return ret;
}

BOOLEAN DPBitmapTestBit(
    DP_BITMAP *     bitmap,
    ULONGLONG       offset
)
{
    BOOLEAN ret;
    ULONG myRegion = 0;
    ULONG myRegionOffset = 0;
    ULONG myByteOffset = 0;
    ULONG myBitPos = 0;

    __try
    {
        //�����Ҫ���Ե�bitmap��Χ���в��ԣ����ȫ��Ϊ0�򷵻�BITMAP_RANGE_CLEAR�����ȫ��Ϊ1���򷵻�BITMAP_RANGE_SET�����Ϊ0��1����򷵻�BITMAP_RANGE_BLEND
        myRegion = (ULONG)(offset / (ULONGLONG)bitmap->regionReferSize);

        myRegionOffset = offset % (ULONGLONG)bitmap->regionReferSize;

        myByteOffset = myRegionOffset / bitmap->byteSize / bitmap->sectorSize;

        myBitPos = (myRegionOffset / bitmap->sectorSize) % bitmap->byteSize;

        if (NULL != *(bitmap->Bitmap + myRegion) && (*(*(bitmap->Bitmap + myRegion) + myByteOffset) &bitmapMask[myBitPos]))
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        ret = FALSE;
    }

    return ret;
}

ULONGLONG
DPBitmapGetNextOffset(
    DP_BITMAP *     bitmap,
    ULONGLONG       start,
    BOOLEAN         usage
)
{
    ULONGLONG nextOffset = 0;
    ULONGLONG offset;

    for (offset = start; offset < bitmap->bitmapReferSize; offset += bitmap->sectorSize)
    {
        if (DPBitmapTestBit(bitmap, offset) != usage)
        {
            nextOffset = offset;
            break;
        }
    }

    return nextOffset;
}