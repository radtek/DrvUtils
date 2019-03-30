#ifndef _RAM_DISK_H
#define _RAM_DISK_H

#include <ntddk.h>
#include <ntdddisk.h>

#include <wdm.h>

#define NT_DEVICE_NAME                  L"\\Device\\WdmRamDisk"
#define DOS_DEVICE_NAME                 L"\\DosDevices\\"
#define RAMDISK_TAG                     'DmaR'  // "RamD"
#define DOS_DEVNAME_LENGTH              (sizeof(DOS_DEVICE_NAME)+sizeof(WCHAR)*10)
#define DRIVE_LETTER_LENGTH             (sizeof(WCHAR)*10)
#define DRIVE_LETTER_BUFFER_SIZE        10
#define DOS_DEVNAME_BUFFER_SIZE         (sizeof(DOS_DEVICE_NAME) / 2) + 10
#define RAMDISK_MEDIA_TYPE              0xF8
#define DEFAULT_SECTOR_SIZE             512
#define DEFAULT_REVERSED_SECTOR         32
#define DEFAULT_DISK_SIZE               (500*1024*1024)     // 500 MB
#define DEFAULT_ROOT_DIR_ENTRIES        512
#define DEFAULT_SECTORS_PER_CLUSTER     2
#define DEFAULT_DRIVE_LETTER            L"Z:"

typedef struct _DISK_INFO {
    ULONG           RootDirEntries;     // ��Ŀ¼��ڴغ�
    ULONG           SectorsPerCluster;  // ÿ�ص�������
    LONGLONG        DiskSize;           // Ramdisk�����ܴ�С
    UNICODE_STRING  DriveLetter;        // ��������, "Z:"
} DISK_INFO, *PDISK_INFO;

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT      SourceDevice;               // RamDisk�豸
    PDEVICE_OBJECT      TargetDevice;               // �ײ�PDO�豸
    PDEVICE_OBJECT      TopDevice;                  // �豸ջ�����豸  
    UNICODE_STRING      SymbolicLink;               // DOS������
    KEVENT              SyncEvent;                  // ͬ��IO����
    IO_REMOVE_LOCK      DeviceRemoveLock;           // �豸ɾ����
    PUCHAR              DiskImage;                  // ����ӳ�����ʼ��ַ
    DISK_GEOMETRY       DiskGeometry;               // Ramdisk���̲���
    DISK_INFO           DiskRegInfo;                // Ramdiskע������
    WCHAR               DriveLetterBuffer[DRIVE_LETTER_BUFFER_SIZE];
    WCHAR               DosDeviceNameBuffer[DOS_DEVNAME_BUFFER_SIZE];
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#pragma pack(1)

//��˵������������
//FAT�ļ�ϵͳ�������������ļ���ͬ���ݽṹ�ں���һ��FAT32�ļ�ϵͳ����������512�ֽ��У�
//90~509�ֽ�Ϊ�������룬��FAT12 / 16����62~509�ֽ�Ϊ�������롣
//ͬʱ��FAT32��������������������������ռ��Ÿ��ӵ��������롣
//һ��FAT��ʹ���ǿ������ļ��ļ�ϵͳ��Ҳ������������롣
typedef struct  _BOOT_SECTOR_FAT32
{
    UCHAR       bsJump[3];          // ��תָ�EB 58 90
    CCHAR       bsOemName[8];       // �ļ�ϵͳ��־�Ͱ汾�ţ�RMDK1.0
    USHORT      bsBytesPerSec;      // ÿ�����ֽ�����512
    UCHAR       bsSecPerClus;       // ÿ����������8
    USHORT      bsResSectors;       // ������������32
    UCHAR       bsFATs;             // FAT�������1
    USHORT      bsRootDirEnts;      // FAT32�������0,FAT12/FAT16Ϊ��Ŀ¼��Ŀ¼�ĸ���
    USHORT      bsSectors;          // FAT32�������0,FAT12/FAT16Ϊ��������
    UCHAR       bsMedia;            // ���ִ洢���ʣ�0xF8��׼ֵ�����ƶ��洢���ʣ����õ� 0xF0
    USHORT      bsFATsecs;          // FAT32����Ϊ0��FAT12/FAT16Ϊһ��FAT����ռ��������
    USHORT      bsSecPerTrack;      // ÿ�ŵ���������32
    USHORT      bsHeads;            // ��ͷ����1
    ULONG       bsHiddenSecs;       // EBR����֮ǰ�����ص�������
    ULONG       bsHugeSectors;      // �ļ�ϵͳ�������� 0x24
    //----------------------------------------------------------------
    //�˲���FAT32���У�������FAT12/16��
    ULONG       bsSecPerFAT;        // ÿ��FAT��ռ��������
    USHORT      bsFAT32Mark;        // ��ǣ�����FAT32 ����
    USHORT      bsFAT32Ver;         // FAT32�汾��0.0��FAT32����
    ULONG       bsRootDirClus;      // ��Ŀ¼���ڵ�һ���صĴغţ�2
    USHORT      bsFsInfoSec;        // FSINFO���ļ�ϵͳ��Ϣ������������1
    USHORT      bsBackBootSec;      // ��������������λ�á�����������������λ���ļ�ϵͳ ��6������
    UCHAR       bsFAT32Ext[12];     // 12�ֽڣ������Ժ�FAT ��չʹ�� 0x1c
    //----------------------------------------------------------------
    UCHAR       bsDriveNumber;      // Drive Number - not used
    UCHAR       bsReserved1;        // Reserved
    UCHAR       bsBootSignature;    // ��չ������־��0x29
    ULONG       bsVolumeID;         // �����кţ�ͨ��Ϊһ�����ֵ��0x12345678
    CCHAR       bsLabel[11];        // ��꣨ASCII�룩����������ļ�ϵͳ��ʱ��ָ���˾� �꣬�ᱣ���ڴ�
    CCHAR       bsFileSystemType[8];// �ļ�ϵͳ��ʽ��ASCII�룬FAT32
    CCHAR       bsReserved2[420];   // �����ֶ�
    UCHAR       bsSig2[2];          // ǩ����־ - 0x55, 0xAA
} BOOT_SECTOR_FAT32, *PBOOT_SECTOR_FAT32;

typedef struct _FSINFO_SECTOR
{
    ULONG       fsExtMark;          // ��չ������־��0x52526141
    CCHAR       fsReserved[480];    // �����ֶΣ�ȫ����0
    ULONG       fsSign;             // FSINFOǩ����0x72724161
    ULONG       fsEmptyClus;        // �ļ�ϵͳ�Ŀմ�����0x000EB772
    ULONG       fsNextValClus;      // ��һ���ôغţ�0x00000015
    CCHAR       fsReserved2[14];    // 14���ֽڣ�δʹ��
    UCHAR       bsSig2[2];          // ǩ����־ - 0x55, 0xAA
} FSINFO_SECTOR, *PFSINFO_SECTOR;

typedef struct  _DIR_ENTRY
{
    UCHAR       deName[8];          // �ļ���
    UCHAR       deExtension[3];     // �ļ���չ��
    UCHAR       deAttributes;       // �ļ�����
    UCHAR       deReserved;         // �����ֶ�
    USHORT      deTime;             // �ļ�ʱ��
    USHORT      deDate;             // �ļ�����
    USHORT      deStartCluster;     // �ļ���ʼ��
    ULONG       deFileSize;         // �ļ���С
} DIR_ENTRY, *PDIR_ENTRY;

#pragma pack()

DRIVER_INITIALIZE DriverEntry;

DRIVER_ADD_DEVICE rdAddDevice;

DRIVER_UNLOAD rdUnload;

DRIVER_DISPATCH rdReadWrite;

DRIVER_DISPATCH rdFlushBuffers;

DRIVER_DISPATCH rdDeviceControl;

DRIVER_DISPATCH rdCreateClose;

DRIVER_DISPATCH rdPnp;

DRIVER_DISPATCH rdPower;

DRIVER_DISPATCH rdSystemControl;

DRIVER_DISPATCH rdScsi;

#endif // _RAM_DISK_H