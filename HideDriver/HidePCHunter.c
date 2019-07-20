#include "HidePCHunter.h"


NTSTATUS GetDriverObjectByName(PDRIVER_OBJECT *DriverObject, WCHAR *DriverName)
{
	PDRIVER_OBJECT TempObject = NULL;
	UNICODE_STRING u_DriverName = { 0 };
	NTSTATUS Status = STATUS_SUCCESS;

	RtlInitUnicodeString(&u_DriverName, DriverName);
	Status = ObReferenceObjectByName(&u_DriverName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, &TempObject);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("��ȡ��������%wsʧ��!�������ǣ�%x!\n", Status));
		*DriverObject = NULL;
		return Status;
	}
	*DriverObject = TempObject;
	return Status;
}

BOOLEAN SupportSEH(PDRIVER_OBJECT DriverObject)
{
	//��Ϊ������������ժ��֮��Ͳ���֧��SEH��
	//������SEH�ַ��Ǹ��ݴ������ϻ�ȡ������ַ���ж��쳣�ĵ�ַ�Ƿ��ڸ�������
	//��Ϊ������û�ˣ��ͻ������
	//ѧϰ����Ϯ�����ķ������ñ��˵�����������������ϵĵ�ַ

	PDRIVER_OBJECT BeepDriverObject = NULL;;
	PLDR_DATA_TABLE_ENTRY LdrEntry = NULL;

	GetDriverObjectByName(&BeepDriverObject, L"\\Driver\\beep");
	if (BeepDriverObject == NULL)
		return FALSE;

	//MiProcessLoaderEntry��������ڲ������Ldr�е�DllBaseȻ��ȥRtlxRemoveInvertedFunctionTable�����ҵ���Ӧ����
	//֮�����Ƴ��������ݲ�������..�������û�е�DllBase��û������SEH������ԭ��û��...
	//����������ϵͳ��Driver\\beep��������...
	LdrEntry = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
	LdrEntry->DllBase = BeepDriverObject->DriverStart;
	ObDereferenceObject(BeepDriverObject);
	return TRUE;
}


PVOID GetProcAddress(WCHAR *FuncName)
{
	UNICODE_STRING u_FuncName = { 0 };
	RtlInitUnicodeString(&u_FuncName, FuncName);
	return MmGetSystemRoutineAddress(&u_FuncName);
}

MIPROCESSLOADERENTRY GetMiProcessLoaderEntry()
{
	/*
	MiProcessLoaderEntry��win10����û�е����ģ������ϲ㺯��MmUnloadSystemImage�ǵ���������
	�������ǿ�������MmUnloadSystemImage����ȡMiProcessLoaderEntry�ĵ�ַ��
	���ö�ջ��
	00 ffffb183`3afa3378 fffff800`3ef9b139 nt!MiProcessLoaderEntry
	01 ffffb183`3afa3380 fffff800`3f0901f0 nt!MiUnloadSystemImage+0x2dd
	02 ffffb183`3afa3500 fffff800`3f090130 nt!MmUnloadSystemImage + 0x20

	ע�⣺win7 win8 ��Щ��������û�е�����
	Callƫ�� = Call��ַ - ��ǰ��ַ - 0x5
	*/
	ULONG_PTR MmUnloadSystemImage = (ULONG_PTR)GetProcAddress(L"MmUnloadSystemImage");
	ULONG_PTR MiUnloadSystemImage = MmUnloadSystemImage + 0x1C + ULONG32TOULONG64(*(ULONG32*)(MmUnloadSystemImage + 0x1C)) + 0x4;
	return (MIPROCESSLOADERENTRY)(MiUnloadSystemImage + 0x2d9 + ULONG32TOULONG64(*(ULONG32*)(MiUnloadSystemImage + 0x2d9)) + 0x4);
}



VOID InitInLoadOrderLinks(PLDR_DATA_TABLE_ENTRY LdrEntry)
{
	InitializeListHead(&LdrEntry->InLoadOrderLinks);
	InitializeListHead(&LdrEntry->InMemoryOrderLinks);
}


VOID HideDriver(PDRIVER_OBJECT DriverObject, PVOID Context, ULONG Count)
{
	MIPROCESSLOADERENTRY MiProcessLoaderEntry = NULL;

	MiProcessLoaderEntry = GetMiProcessLoaderEntry();
	if (MiProcessLoaderEntry == NULL)
		return;
	KdPrint(("MiProcessLoaderEntry:%llX", (ULONG_PTR)MiProcessLoaderEntry));
	SupportSEH(DriverObject);
	MiProcessLoaderEntry(DriverObject->DriverSection, 0);
	InitInLoadOrderLinks((PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection);
	
	//pchunter�޸�DriverSection����
	//DriverObject->DriverSection = NULL;
	DriverObject->DriverStart = NULL;
	DriverObject->DriverSize = 0;
	DriverObject->DriverUnload = NULL;
	DriverObject->DriverInit = NULL;
	DriverObject->DeviceObject = NULL;
}

NTSTATUS HidePCHDriverDepsSelf(PDRIVER_OBJECT pDriverObject)
{
	//Ϊ�˸���������Ӧ����PCHunter���������������Ǽ��ص�ʱ�����ж�PCHunter�������Ƿ����,
	//���PCHunter�������Ѿ������أ���ô���Ƕ����������أ��Զ��EAC��ɨ��
	//ʹ�������druver_section����λPCHunter��driver_section
	PAGED_CODE()
	NTSTATUS				Status = STATUS_SUCCESS;
	PLDR_DATA_TABLE_ENTRY	SelfSection = (PLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
	PLDR_DATA_TABLE_ENTRY	TempSection;
	UNICODE_STRING			usPcHunter;
	PLDR_DATA_TABLE_ENTRY	PcHunterSection = NULL;
	RtlInitUnicodeString(&usPcHunter, L"PCHUNTER*");
	TempSection = SelfSection;
	__try {
		do
		{
			if (SelfSection->BaseDllName.Buffer != NULL)
			{
				if (FsRtlIsNameInExpression(&usPcHunter, &SelfSection->BaseDllName, TRUE, NULL))
				{
					PcHunterSection = SelfSection;
					break;
				}
				SelfSection = (PLDR_DATA_TABLE_ENTRY)(SelfSection->InLoadOrderLinks.Blink);
			}
		} while (SelfSection->InLoadOrderLinks.Blink != (PLIST_ENTRY)(TempSection));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return STATUS_UNSUCCESSFUL;
	}

	if (PcHunterSection)
	{
		MIPROCESSLOADERENTRY MiProcessLoaderEntry = NULL;
		MiProcessLoaderEntry = GetMiProcessLoaderEntry();
		if (MiProcessLoaderEntry == NULL)
			return STATUS_UNSUCCESSFUL;
		KdPrint(("%llX", (ULONG_PTR)MiProcessLoaderEntry));
		SupportSEH(pDriverObject);
		MiProcessLoaderEntry(PcHunterSection, 0);
		InitInLoadOrderLinks((PLDR_DATA_TABLE_ENTRY)PcHunterSection);
	}
	return Status;
}