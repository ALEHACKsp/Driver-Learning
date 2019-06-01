/**************************************************************
 *  Filename:    DriverEntry.c
 *
 *  Description: �ҵĵ�һ������
 *
 *  @author:     XAGU
 **************************************************************/


#include <ntddk.h>
#include "DriverEntry.h"

#define DEV_DEVICE L"\\Device\\FirstDevice"
#define LINK_NAME L"\\??\\FirstDevice"

void MyDriverUnload(PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING usSymName;
	RtlInitUnicodeString(&usSymName, LINK_NAME);
	if (pDriverObject->DeviceObject!=NULL)
	{
		IoDeleteSymbolicLink(&usSymName);
		IoDeleteDevice(pDriverObject->DeviceObject);
		KdPrint(("����ж�سɹ���"));
	}
}

NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	UNICODE_STRING usDevName;
	UNICODE_STRING usSymName;


	RtlInitUnicodeString(&usDevName, DEV_DEVICE);
	status = IoCreateDevice(pDriverObject, 0, &usDevName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevObj);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	pDevObj->Flags |= DO_BUFFERED_IO;
	RtlInitUnicodeString(&usSymName, LINK_NAME);
	status = IoCreateSymbolicLink(&usSymName, &usDevName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	return STATUS_SUCCESS;
}

NTSTATUS CreateCompleteRoutine(PDEVICE_OBJECT pDriverObj, PIRP pIrp)
{
	NTSTATUS status;
	status = STATUS_SUCCESS;
	KdPrint(("Create"));
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS CloseCompleteRoutine(PDEVICE_OBJECT pDriverObj, PIRP pIrp)
{
	NTSTATUS status;
	status = STATUS_SUCCESS;
	KdPrint(("Close"));
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS WriteCompleteRoutine(PDEVICE_OBJECT pDriverObj, PIRP pIrp)
{
	NTSTATUS status;
	status = STATUS_SUCCESS;
	KdPrint(("Write"));
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS ReadCompleteRoutine(PDEVICE_OBJECT pDriverObj, PIRP pIrp)
{
	NTSTATUS status;
	status = STATUS_SUCCESS;
	KdPrint(("Read"));
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status;
	KdPrint(("�������سɹ���"));
	status = CreateDevice(pDriverObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("����deviceʧ�ܣ�"));
	}
	else
	{
		KdPrint(("����device�ɹ���"));
		KdPrint(("%wZ",pRegistryPath));
	}

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCompleteRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCompleteRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = WriteCompleteRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = ReadCompleteRoutine;
	pDriverObject->DriverUnload = MyDriverUnload;
	return STATUS_SUCCESS;
}