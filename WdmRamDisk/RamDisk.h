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

//�������̹��ܺ�
#define IOCTL_VIRTUAL_DISK_BASE             FILE_DEVICE_VIRTUAL_DISK
#define IOCTL_VIRTUAL_DISK_CREATE_DISK      CTL_CODE(IOCTL_VIRTUAL_DISK_BASE, 0x0000, METHOD_BUFFERED, FILE_ANY_ACCESS)

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