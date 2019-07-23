#include "Common.h"

VOID FilterProcessNotify(
	_In_ HANDLE ParentId,
	_In_ HANDLE ProcessId,
	_In_ BOOLEAN Create
)
{
	if (Create&&ProcessId != (HANDLE)4&& ProcessId != (HANDLE)0)
	{
		NTSTATUS			Status = STATUS_SUCCESS;
		PEPROCESS			Eprocess = NULL;
		PVOID				ProcessBaseAddr = NULL;
		KAPC_STATE			ApcState = { 0 };
		Status = PsLookupProcessByProcessId(ProcessId, &Eprocess);
		if (Eprocess == NULL)
		{
			KdPrint(("PsLookupProcessByProcessId Failed !"));
			return;
		}
		ObDereferenceObject(Eprocess);
		ProcessBaseAddr = PsGetProcessSectionBaseAddress(Eprocess);
		if (ProcessBaseAddr == NULL)
		{
			KdPrint(("PsGetProcessSectionBaseAddress Failed !"));
			return;
		}
		//���ӽ���
		RtlZeroMemory(&ReadCode, sizeof(ReadCode));
		KeStackAttachProcess(Eprocess, &ApcState);
		//����R3
		__try
		{
			ProbeForRead((PVOID)((ULONG_PTR)ProcessBaseAddr+Offset), sizeof(ReadCode), 1);
			RtlCopyMemory((PVOID)ReadCode, (PVOID)((ULONG_PTR)ProcessBaseAddr + Offset), sizeof(ReadCode));
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			KdPrint(("mem is error !"));
		}
		KeUnstackDetachProcess(&ApcState);
		KdPrint(("%X", (ULONG_PTR)ProcessBaseAddr));
		for (size_t i = 0; i < sizeof(ReadCode); i++)
		{
			KdPrint(("Read %X", ReadCode[i]));
		}
	}
	return;
}

VOID DriverUnLoad(PDRIVER_OBJECT pDriverObject)
{
	PsSetCreateProcessNotifyRoutine(FilterProcessNotify, TRUE);
	KdPrint(("����ж�سɹ���"));
}


DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	NTSTATUS				Status;
	KdPrint(("�������سɹ���"));
	Status = PsSetCreateProcessNotifyRoutine(FilterProcessNotify, FALSE);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("PsSetCreateProcessNotifyRoutine Failed"));
		return STATUS_UNSUCCESSFUL;
	}
	pDriverObject->DriverUnload = DriverUnLoad;
	return STATUS_SUCCESS;
}