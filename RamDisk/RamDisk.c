#include "RamDisk.h"


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING  RegistryPath
)
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status = STATUS_SUCCESS;

    WDF_DRIVER_CONFIG_INIT(&config, rdAddDevice);
    config.EvtDriverUnload = rdUnload;

    status = WdfDriverCreate(DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES, 
        &config,
        WDF_NO_HANDLE);

    if (!NT_SUCCESS(status))
    {
        MyDbgPrint("[RamDisk]WdfDriverCreate failed.");
    }

    return status;
}

NTSTATUS
rdAddDevice(
    IN WDFDRIVER Driver,
    IN OUT PWDFDEVICE_INIT DeviceInit
)
{
    NTSTATUS                status;
    WDF_OBJECT_ATTRIBUTES   devAttr;
    WDFDEVICE               device;
    WDF_OBJECT_ATTRIBUTES   queueAttr;
    WDF_IO_QUEUE_CONFIG     queueConfig;
    PDEVICE_EXTENSION       pDevExt;
    PQUEUE_EXTENSION        pQueueExt;
    WDFQUEUE                queue;
    DECLARE_CONST_UNICODE_STRING(devName, RAMDISK_DEVICE_NAME);

    PAGED_CODE();

    UNREFERENCED_PARAMETER(Driver);

    //����WDF�����豸
    status = WdfDeviceInitAssignName(DeviceInit, &devName);
    if (!NT_SUCCESS(status))
    {
        MyDbgPrint("[RamDisk]WdfDeviceInitAssignName failed.");
        return status;
    }

    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_DISK);
    WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);
    WdfDeviceInitSetExclusive(DeviceInit, FALSE);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&devAttr, DEVICE_EXTENSION);

    devAttr.EvtCleanupCallback = rdEvtCleanup;

    status = WdfDeviceCreate(&DeviceInit, &devAttr, &device);
    if (!NT_SUCCESS(status))
    {
        MyDbgPrint("[RamDisk]WdfDeviceCreate failed.");
        return status;
    }

    pDevExt = DeviceGetExtention(device);

    //��ʼ���豸IO����
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchSequential);

    queueConfig.EvtIoDeviceControl = rdEvtIoDeviceControl;
    queueConfig.EvtIoRead = rdEvtIoRead;
    queueConfig.EvtIoWrite = rdEvtIoWrite;

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queueAttr, QUEUE_EXTENSION);

    status = WdfIoQueueCreate(device,
        &queueConfig,
        &queueAttr,
        &queue);
    if (!NT_SUCCESS(status))
    {
        MyDbgPrint("[RamDisk]WdfIoQueueCreate failed.");
        return status;
    }

    pQueueExt = QueueGetExtension(queue);

    pQueueExt->DeviceExtension = pDevExt;

    //��ʼ������

    return status;
}

VOID
rdUnload(
    IN WDFDRIVER Driver
)
{

}

VOID
rdEvtCleanup(
    IN WDFOBJECT Device
)
{

}

VOID
rdEvtIoRead(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t Length
)
{

}

VOID
rdEvtIoWrite(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t Length
)
{

}

VOID
rdEvtIoDeviceControl(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t OutputBufferLength,
    IN size_t InputBufferLength,
    IN ULONG IoControlCode
)
{

}

VOID
rdFormatDisk(
    IN PDEVICE_EXTENSION DevExt
)
{
    PBOOT_SECTOR_FAT32  bootSector = (PBOOT_SECTOR_FAT32)DevExt->DiskImage;
    PFSINFO_SECTOR      fsInfoSector = (PFSINFO_SECTOR)(DevExt->DiskImage + DEFAULT_SECTOR_SIZE);
    PUCHAR              firstFatSector; //FAT����ʼ����
    PDIR_ENTRY          rootDir;        //��Ŀ¼��������
    ULONG               fatSecCnt;      //��������

    PAGED_CODE();
    ASSERT(sizeof(BOOT_SECTOR_FAT32) != DEFAULT_SECTOR_SIZE);
    ASSERT(sizeof(FSINFO_SECTOR) != DEFAULT_SECTOR_SIZE);
    ASSERT(bootSector != NULL);

    //��ʽ����������
    RtlZeroMemory(DevExt->DiskRegInfo.DiskSize, DEFAULT_SECTOR_SIZE * DEFAULT_REVERSED_SECTOR);

    //��ʼ���������в���
    //Ӳ������ �� ����/�ŵ��� * ������ * ÿ�����ֽ��� * ��ͷ��
    DevExt->DiskGeometry.BytesPerSector     = DEFAULT_SECTOR_SIZE; //ÿ�������ֽ���
    DevExt->DiskGeometry.SectorsPerTrack    = 32; //ÿ���ŵ�������
    DevExt->DiskGeometry.TracksPerCylinder  = 1; //ÿ������ŵ���
    DevExt->DiskGeometry.MediaType          = RAMDISK_MEDIA_TYPE;
    DevExt->DiskGeometry.Cylinders.QuadPart = DevExt->DiskRegInfo.DiskSize / (DEFAULT_SECTOR_SIZE * 32 * 1); //������

    MyDbgPrint(
        "[RamDisk]Cylinders: %I64d\n TracksPerCylinder: %lu\n SectorsPerTrack: %lu\n BytesPerSector: %lu\n",
        DevExt->DiskGeometry.Cylinders.QuadPart, DevExt->DiskGeometry.TracksPerCylinder,
        DevExt->DiskGeometry.SectorsPerTrack, DevExt->DiskGeometry.BytesPerSector
        );

    //��ʼ��DBR����
    bootSector->bsJump[0] = 0xEB;
    bootSector->bsJump[1] = 0x58;
    bootSector->bsJump[2] = 0x90;

    bootSector->bsOemName[0] = 'R';
    bootSector->bsOemName[1] = 'M';
    bootSector->bsOemName[2] = 'D';
    bootSector->bsOemName[3] = 'K';
    bootSector->bsOemName[4] = '1';
    bootSector->bsOemName[5] = '.';
    bootSector->bsOemName[6] = '0';
    bootSector->bsOemName[7] = '\0';

    bootSector->bsBytesPerSec   = (USHORT)DevExt->DiskGeometry.BytesPerSector;
    bootSector->bsSecPerClus    = 8;
    bootSector->bsResSectors    = (USHORT)DEFAULT_REVERSED_SECTOR;
    bootSector->bsFATs          = 1;
    bootSector->bsMedia         = (UCHAR)DevExt->DiskGeometry.MediaType;
    bootSector->bsSecPerTrack   = (USHORT)DevExt->DiskGeometry.SectorsPerTrack;
    bootSector->bsHeads         = 1;
    bootSector->bsHugeSectors   = (ULONG)(DevExt->DiskRegInfo.DiskSize / DevExt->DiskGeometry.BytesPerSector);
    bootSector->bsSecPerFAT     = (bootSector->bsHugeSectors - bootSector->bsResSectors) / bootSector->bsSecPerClus + 2;
    bootSector->bsRootDirClus   = 2;
    bootSector->bsFsInfoSec     = 1;
    bootSector->bsBackBootSec   = 6;
    bootSector->bsBootSignature = 0x29;
    bootSector->bsVolumeID      = 0x12345678;

    bootSector->bsLabel[0]  = 'R';
    bootSector->bsLabel[1]  = 'a';
    bootSector->bsLabel[2]  = 'm';
    bootSector->bsLabel[3]  = 'D';
    bootSector->bsLabel[4]  = 'i';
    bootSector->bsLabel[5]  = 's';
    bootSector->bsLabel[6]  = 'k';
    bootSector->bsLabel[7]  = '\0';
    bootSector->bsLabel[8]  = '\0';
    bootSector->bsLabel[9]  = '\0';
    bootSector->bsLabel[10] = '\0';

    bootSector->bsFileSystemType[0] = 'F';
    bootSector->bsFileSystemType[1] = 'A';
    bootSector->bsFileSystemType[2] = 'T';
    bootSector->bsFileSystemType[3] = '3';
    bootSector->bsFileSystemType[4] = '2';
    bootSector->bsFileSystemType[5] = '\0';
    bootSector->bsFileSystemType[6] = '\0';
    bootSector->bsFileSystemType[7] = '\0';

    bootSector->bsSig2[0] = 0x55;
    bootSector->bsSig2[1] = 0xAA;

    //��ʼ��FSINFO����
    fsInfoSector->fsExtMark     = 0x52526141;
    fsInfoSector->fsSign        = 0x72724161;
    fsInfoSector->fsEmptyClus   = 0xFFFFFFFF;
    fsInfoSector->fsNextValClus = 0x2;

    fsInfoSector->bsSig2[0] = 0x55;
    fsInfoSector->bsSig2[1] = 0xAA;

    //��ʼ�����ݷ���
    RtlCopyMemory((bootSector + bootSector->bsBackBootSec), bootSector, DevExt->DiskGeometry.BytesPerSector);
    RtlCopyMemory((bootSector + bootSector->bsBackBootSec + 1), bootSector + 1, DevExt->DiskGeometry.BytesPerSector);

    //��ʼ���ļ�Ŀ¼��    
    firstFatSector = (PUCHAR)(bootSector + bootSector->bsResSectors);

    RtlZeroMemory(firstFatSector, bootSector->bsSecPerFAT * DevExt->DiskGeometry.BytesPerSector);

    firstFatSector[0] = (UCHAR)DevExt->DiskGeometry.MediaType;
    firstFatSector[1] = 0xFF;
    firstFatSector[2] = 0xFF;
    firstFatSector[3] = 0xFF;

    //��ʼ����Ŀ¼��һ����Ŀ¼��
    rootDir = (PDIR_ENTRY)(bootSector + bootSector->bsResSectors + bootSector->bsSecPerFAT);

    rootDir->deName[0] = 'R';
    rootDir->deName[1] = 'A';
    rootDir->deName[2] = 'M';
    rootDir->deName[3] = 'D';
    rootDir->deName[4] = 'I';
    rootDir->deName[5] = 'S';
    rootDir->deName[6] = 'K';
    rootDir->deName[7] = '1';

    rootDir->deExtension[0] = 'I';
    rootDir->deExtension[1] = 'V';
    rootDir->deExtension[2] = 'E';

    rootDir->deAttributes = 0x08;
}