#ifndef _RAM_DISK_H
#define _RAM_DISK_H

#include <ntddk.h>
#include <ntdddisk.h>

#include <wdm.h>

#define NT_DEVICE_NAME                  L"\\Device\\WdmRamDisk"
#define DOS_DEVICE_NAME                 L"\\DosDevices\\"
#define SDDL_ALL                        L"D:P(A;;GA;;;WD)"
#define RAMDISK_TAG                     'DmaR'  // "RamD"
#define DOS_DEVNAME_LENGTH              (sizeof(DOS_DEVICE_NAME)+sizeof(WCHAR)*10)
#define DRIVE_LETTER_LENGTH             (sizeof(WCHAR)*10)
#define DRIVE_LETTER_BUFFER_SIZE        10
#define DOS_DEVNAME_BUFFER_SIZE         (sizeof(DOS_DEVICE_NAME) / 2) + 10
#define RAMDISK_MEDIA_TYPE              0xF8
#define DEFAULT_SECTOR_SIZE             512
#define DEFAULT_REVERSED_SECTOR         32
#define DEFAULT_DISK_SIZE               (100*1024*1024)     // 100 MB
#define DEFAULT_ROOT_DIR_ENTRIES        512
#define DEFAULT_SECTORS_PER_CLUSTER     2
#define DEFAULT_DRIVE_LETTER            L"Z:"

const GUID DECLSPEC_SELECTANY GUID_CLASS_WDMRAMDISK =
{ 0x2145ea58, 0x1213, 0x86a2, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88} };


typedef struct _DISK_INFO {
    UCHAR           PartitionType;      // ��������
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

NTSTATUS rdCreateRamDisk(
    IN PDEVICE_EXTENSION deviceExtension
);

NTSTATUS
rdQueryDiskParameter(
    IN PWSTR RegistryPath,
    IN PDISK_INFO DiskInfo
);

#endif // _RAM_DISK_H