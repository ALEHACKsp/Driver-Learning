#include <ntifs.h>


NTSTATUS ReadWriteMem()
{
	NTSTATUS Status;
	HANDLE hProcess;
	OBJECT_ATTRIBUTES ObjAttr;
	CLIENT_ID ClientId;
	PVOID AllocateAddress;
	SIZE_T RegionSize = 0xff;
	RtlZeroMemory(&ObjAttr, sizeof(OBJECT_ATTRIBUTES));
	ClientId.UniqueProcess = 2764;
	ClientId.UniqueThread = 0;
	Status = ZwOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &ObjAttr, &ClientId);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("���̴�ʧ��! Code:%X",Status));
		return Status;
	}
	KdPrint(("���̴򿪳ɹ�! Process:%llX", hProcess));

	ZwAllocateVirtualMemory(hProcess, &AllocateAddress, 0, &RegionSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("�����ڴ�ʧ��! Code:%X", Status));
		return Status;
	}
	KdPrint(("�����ڴ�ɹ�! Address:%ld��Address:%lx,Address:%X,Size:%x", AllocateAddress, AllocateAddress, AllocateAddress,RegionSize));

	ZwClose(hProcess);
	return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	KdPrint(("����ж�سɹ�!"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
	ReadWriteMem();
	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}