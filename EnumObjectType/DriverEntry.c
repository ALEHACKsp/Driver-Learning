#include <ntifs.h>

NTSTATUS DriverUnLoad(PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("����ж�سɹ���"));
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	ULONG_PTR				uIndex;
	PVOID					pfn_ObGetObjectType;
	ULONG_PTR				*ObTypeIndexTable;
	UNICODE_STRING			str_func_name;
	RtlInitUnicodeString(&str_func_name, L"ObGetObjectType");
	pfn_ObGetObjectType = MmGetSystemRoutineAddress(&str_func_name);
	if (!MmIsAddressValid(pfn_ObGetObjectType))
	{
		KdPrint(("pfn_ObGetObjectType is null"));
	}
	else
	{
		KdPrint(("pfn_ObGetObjectType + 15 : %X", (ULONG_PTR)pfn_ObGetObjectType + 15));
		ObTypeIndexTable = *(ULONG_PTR**)((ULONG_PTR)pfn_ObGetObjectType + 15);
		uIndex = 2;
		while (ObTypeIndexTable[uIndex])
		{
			KdPrint(("ObTypeIndexTable[%d]:%wZ",uIndex, ObTypeIndexTable[uIndex]+8));
			uIndex++;
		}
	}
	pDriverObject->DriverUnload = DriverUnLoad;
	KdPrint(("�������سɹ���"));
	return STATUS_SUCCESS;
}