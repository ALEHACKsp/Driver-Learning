#include "HideProcess.h"

VOID DriverUnLoad(PDRIVER_OBJECT pDriverObject)
{
	UnHideProcess((HANDLE)1192);
	KdPrint(("����ж�سɹ���"));
}

DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	NTSTATUS				Status;
	KdPrint(("�������سɹ���"));
	HideProcess((HANDLE)1192);
	pDriverObject->DriverUnload = DriverUnLoad;
	return STATUS_SUCCESS;
}