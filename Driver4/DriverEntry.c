/**************************************************************
 *  Filename:    DriverEntry.c
 *
 *  Description: kernel�µ��ļ�����
 *
 *  @author:     XAGU
 **************************************************************/

#include <ntddk.h>

#define DEV_DEVICE L"\\Device\\FirstDevice"
#define LINK_NAME L"\\??\\FirstDevice"
#define BUFFER_SIZE 1024

NTSTATUS MyCreateFile()
{
	NTSTATUS Status;
	HANDLE hFile;
	OBJECT_ATTRIBUTES FileObjAttr;
	UNICODE_STRING usFileName;
	IO_STATUS_BLOCK IoStatusBlock;
	UNICODE_STRING usBuffer;
	//PUCHAR pBuffer;
	memset(&FileObjAttr, 0, sizeof(OBJECT_ATTRIBUTES));
	RtlInitUnicodeString(&usFileName, L"\\??\\C:\\test.txt");
	InitializeObjectAttributes(&FileObjAttr,&usFileName,OBJ_CASE_INSENSITIVE,NULL,NULL);
	Status = ZwCreateFile(&hFile,
		GENERIC_ALL,
		&FileObjAttr,
		&IoStatusBlock,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
		);

	if (!NT_SUCCESS(Status))
	{
		KdPrint(("�����ļ�ʧ�ܣ�"));
		return Status;
	}
	KdPrint(("�����ļ��ɹ���"));

	RtlInitUnicodeString(&usBuffer, L"China no one !");

	//pBuffer = (PUCHAR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	//RtlFillMemory(pBuffer, BUFFER_SIZE, 'A');
	Status = ZwWriteFile(hFile, NULL, NULL, NULL, &IoStatusBlock, usBuffer.Buffer, usBuffer.Length, NULL, NULL);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("д���ļ�ʧ�ܣ�%x", Status));
		ZwClose(hFile);
		return Status;
	}
	KdPrint(("д���ļ��ɹ���%x", Status));
	ZwClose(hFile);
	return Status;
}

void MyDriverUnload(PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("����ж�سɹ���"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	KdPrint(("�������سɹ���"));
	MyCreateFile();
	pDriverObject->DriverUnload = MyDriverUnload;
	return STATUS_SUCCESS;
}