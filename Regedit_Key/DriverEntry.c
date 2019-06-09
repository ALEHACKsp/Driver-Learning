#include <ntddk.h>

NTSTATUS CreateKey()
{
	NTSTATUS Status;
	HANDLE hRegister;
	OBJECT_ATTRIBUTES ObjAttr;
	UNICODE_STRING usKeyName,usValueKey;
	ULONG Disposition;
	DWORD64 value = 9;
	RtlZeroMemory(&ObjAttr, sizeof(OBJECT_ATTRIBUTES));
	RtlInitUnicodeString(&usKeyName, L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet001\\services\\MyKey");
	InitializeObjectAttributes(&ObjAttr, &usKeyName, OBJ_CASE_INSENSITIVE, NULL, NULL);
	Status = ZwCreateKey(&hRegister, KEY_ALL_ACCESS, &ObjAttr, 0, NULL, REG_OPTION_NON_VOLATILE, &Disposition);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("����ע���ʧ��:%X",Status));
		return Status;
	}
	if (Disposition == REG_CREATED_NEW_KEY)
	{
		KdPrint(("����ע���ɹ���"));
	}
	else
	{
		KdPrint(("��ע���ɹ���"));
	}

	RtlInitUnicodeString(&usValueKey, L"Name");
	ZwSetValueKey(hRegister, &usValueKey, NULL, REG_DWORD, &value, sizeof(DWORD32));
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("File Code:%X", Status));
		return Status;
	}
	ZwClose(hRegister);
	return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	KdPrint(("����ж�سɹ�!"));
}


NTSTATUS OpenKey()
{
	NTSTATUS Status;
	HANDLE hRegister;
	OBJECT_ATTRIBUTES ObjAttr;
	UNICODE_STRING usKeyName;
	RtlZeroMemory(&ObjAttr, sizeof(OBJECT_ATTRIBUTES));
	RtlInitUnicodeString(&usKeyName, L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet001\\services\\MyKey");
	InitializeObjectAttributes(&ObjAttr, &usKeyName, OBJ_CASE_INSENSITIVE, NULL, NULL);
	Status = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &ObjAttr);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("��ע���ʧ��:%X", Status));
		return Status;
	}
	KdPrint(("��ע���ɹ���"));
	ZwClose(hRegister);
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
	pDriver->DriverUnload = DriverUnload;
	KdPrint(("�������سɹ�!"));
	KdPrint(("%wZ", pRegPath));
	//����ע���
	CreateKey();
	//��ע���
	// OpenKey();
	return STATUS_SUCCESS;
}