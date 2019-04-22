#ifndef _SHADOW_VOLUME_H
#define _SHADOW_VOLUME_H

#include <ntddk.h>
#include <ntddvol.h>

#include "DiskBitmap.h"

#define DEFAULT_SECTOR_SIZE 512

typedef struct  _BOOT_SECTOR_FAT
{
    UCHAR       bsJump[3];                  // ��תָ�EB 58 90
    CCHAR       bsOemName[8];               // �ļ�ϵͳ��־�Ͱ汾��
    USHORT      bsBytesPerSec;              // ÿ�����ֽ���
    UCHAR       bsSecPerClus;               // ÿ��������
    USHORT      bsResSectors;               // ����������
    UCHAR       bsFATs;                     // FAT�����
    USHORT      bsRootDirEnts;              // ��Ŀ¼��Ŀ¼�ĸ���
    USHORT      bsSectors;                  // ��������
    UCHAR       bsMedia;                    // �洢���ʣ�0xF8��׼ֵ
    USHORT      bsFATsecs;                  // FAT����ռ��������
    USHORT      bsSecPerTrack;              // ÿ�ŵ�������
    USHORT      bsHeads;                    // ��ͷ��
    ULONG       bsHiddenSecs;               // EBR����֮ǰ�����ص�������
    ULONG       bsHugeSectors;              // �ļ�ϵͳ��������
    UCHAR       bsDriveNumber;              // Drive Number - not used
    UCHAR       bsReserved1;                // Reserved
    UCHAR       bsBootSignature;            // ��չ������־
    ULONG       bsVolumeID;                 // �����кţ�ͨ��Ϊһ�����ֵ��0x12345678
    CCHAR       bsLabel[11];                // ��꣨ASCII�룩
    CCHAR       bsFileSystemType[8];        // �ļ�ϵͳ��ʽ��ASCII�룬FAT12 �� FAT16
    CCHAR       bsReserved2[448];           // Reserved
    UCHAR       bsSig2[2];                  // ǩ����־ - 0x55, 0xAA
} BOOT_SECTOR_FAT, *PBOOT_SECTOR_FAT;

typedef struct  _BOOT_SECTOR_FAT32 {
    UCHAR       bsJump[3];                  // ��תָ�EB 58 90
    CCHAR       bsOemName[8];               // �ļ�ϵͳ��־�Ͱ汾�ţ�RMDK1.0
    USHORT      bsBytesPerSec;              // ÿ�����ֽ�����512
    UCHAR       bsSecPerClus;               // ÿ����������8
    USHORT      bsResSectors;               // ������������32
    UCHAR       bsFATs;                     // FAT�������1
    USHORT      bsRootDirEnts;              // FAT32�������0,FAT12/FAT16Ϊ��Ŀ¼��Ŀ¼�ĸ���
    USHORT      bsSectors;                  // FAT32�������0,FAT12/FAT16Ϊ��������
    UCHAR       bsMedia;                    // ���ִ洢���ʣ�0xF8��׼ֵ�����ƶ��洢���ʣ����õ� 0xF0
    USHORT      bsFATsecs;                  // FAT32����Ϊ0��FAT12/FAT16Ϊһ��FAT����ռ��������
    USHORT      bsSecPerTrack;              // ÿ�ŵ���������32
    USHORT      bsHeads;                    // ��ͷ����1
    ULONG       bsHiddenSecs;               // EBR����֮ǰ�����ص�������
    ULONG       bsHugeSectors;              // �ļ�ϵͳ�������� 0x24
    //----------------------------------------------------------------
    //�˲���FAT32���У�������FAT12/16��
    ULONG       bsSecPerFAT;                // ÿ��FAT��ռ��������
    USHORT      bsFAT32Mark;                // ��ǣ�����FAT32 ����
    USHORT      bsFAT32Ver;                 // FAT32�汾��0.0��FAT32����
    ULONG       bsRootDirClus;              // ��Ŀ¼���ڵ�һ���صĴغţ�2
    USHORT      bsFsInfoSec;                // FSINFO���ļ�ϵͳ��Ϣ������������1
    USHORT      bsBackBootSec;              // ��������������λ�á�����������������λ���ļ�ϵͳ ��6������
    UCHAR       bsFAT32Ext[12];             // 12�ֽڣ������Ժ�FAT ��չʹ�� 0x1c
    //----------------------------------------------------------------
    UCHAR       bsDriveNumber;              // Drive Number - not used
    UCHAR       bsReserved1;                // Reserved
    UCHAR       bsBootSignature;            // ��չ������־��0x29
    ULONG       bsVolumeID;                 // �����кţ�ͨ��Ϊһ�����ֵ��0x12345678
    CCHAR       bsLabel[11];                // ��꣨ASCII�룩����������ļ�ϵͳ��ʱ��ָ���˾� �꣬�ᱣ���ڴ�
    CCHAR       bsFileSystemType[8];        // �ļ�ϵͳ��ʽ��ASCII�룬FAT32
    CCHAR       bsReserved2[420];           // �����ֶ�
    UCHAR       bsSig2[2];                  // ǩ����־ - 0x55, 0xAA
} BOOT_SECTOR_FAT32, *PBOOT_SECTOR_FAT32;

typedef struct _BOOT_SECTOR_NTFS {
    UCHAR		bsJump[3];					// jump code
    UCHAR		bsFSID[8];					// 'NTFS '
    USHORT		bsBytesPerSector;			// Bytes Per Sector
    UCHAR		bsSectorsPerCluster;		// Sectors Per Cluster
    USHORT		bsReservedSectors;			// Reserved Sectors
    UCHAR		bsMbz1;						// always 0	
    USHORT		bsMbz2;						// always 0
    USHORT		bsReserved1;				// not used by NTFS
    UCHAR		bsMediaDesc;			    // Media Descriptor
    USHORT		bsMbz3;						// always 0
    USHORT		bsSectorsPerTrack;			// Sectors Per Track
    USHORT		bsHeads;					// Number Of Heads
    ULONG		bsHiddenSectors;			// Hidden Sectors
    ULONG		bsReserved2[2];				// not used by NTFS
    ULONGLONG	bsTotalSectors;				// Total Sectors
    ULONGLONG	bsMftStartLcn;				// Logical Cluster Number for the file $MFT
    ULONGLONG	bsMft2StartLcn;				// Logical Cluster Number for the file $MFTMirr
    ULONG       bsClustersPerFileSeg;       // Clusters Per File Record Segment
    UCHAR       bsClustersPerIndexBuf;      // Clusters Per Index Buffer
    UCHAR       bsReserved3[3];             // not used by NTFS
    ULONGLONG   bsVolumeSerialNum;          // Volume Serial Number
    ULONG       bsCheckSum;                 // Checksum
} BOOT_SECTOR_NTFS, *PBOOT_SECTOR_NTFS;

typedef struct _DEVICE_EXTENSION {
    WCHAR               VolumeLetter;       // ���̾��̷�
    ULONG               EnableProtect;      // �Ƿ�������
    ULONG               SectorsPerCluster;  // ÿ�ش�С
    ULONG               BytesPerSector;     // ÿ������С
    LARGE_INTEGER       VolumeTotalSize;    // ���̾��ܴ�С
    PDEVICE_OBJECT      FilterDevice;       // ���̾�����豸
    PDEVICE_OBJECT      TargetDevice;       // ���̾��豸
    PDEVICE_OBJECT      LowerDevice;        // ת������һ���豸
    ULONG               InitCompleted;      // ��ɳ�ʼ��
    PDISK_BITMAP        DiskBitmap;         // ����λͼ
    HANDLE              FileDump;           // ����ת���ļ����
    LIST_ENTRY          RequestList;        // ���������
    KSPIN_LOCK          RequestLock;        // ������з�����
    KEVENT              RequestEvent;       // �������ͬ���¼�
    PVOID               ThreadHandle;       // ���������¼��߳̾��
    BOOLEAN             ThreadTerminate;    // �߳̽�����־
    ULONG               DiskPagingCount;    // ���̿�����ҳ����
    KEVENT              DiskPagingEvent;    // ���̷�ҳ�ȴ��¼�
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _COMPLETION_CONTEXT {
    PKEVENT             SyncEvent;
    PDEVICE_EXTENSION   DeviceExtension;
} COMPLETION_CONTEXT, *PCOMPLETION_CONTEXT;

DRIVER_INITIALIZE DriverEntry;

DRIVER_REINITIALIZE svReinitializationRoutine;

DRIVER_ADD_DEVICE svAddDevice;

DRIVER_UNLOAD svUnload;

DRIVER_DISPATCH svDispatchGeneral;

DRIVER_DISPATCH svDispatchRead;

DRIVER_DISPATCH svDispatchWrite;

DRIVER_DISPATCH svDispatchDeviceControl;

DRIVER_DISPATCH svDispatchPnp;

DRIVER_DISPATCH svDispatchPower;

KSTART_ROUTINE svReadWriteThread;

NTSTATUS
SendToLowerDevice(
    IN PDEVICE_OBJECT LowerDevice,
    IN PIRP Irp
);

NTSTATUS
WaitOnLowerDevice(
    IN PDEVICE_OBJECT LowerDevice,
    IN PIRP Irp
);

NTSTATUS
svDispatchPnpCompleteRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
);

VOID
IoAdjustPagingCount(
    IN volatile PULONG PagingCount,
    IN BOOLEAN InPath
);

WCHAR
ToUpperLetter(
    IN WCHAR Letter
);

NTSTATUS
QueryVolumeInformation(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG SectorsPerCluster,
    OUT PULONG SizePerSector,
    OUT PLARGE_INTEGER VolumeTotalSize
);

NTSTATUS
svDispatchDeviceControlCompleteRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
);

#endif // _SHADOW_VOLUME_H
