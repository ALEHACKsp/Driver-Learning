#include <ntifs.h>

#pragma pack(1) //д����ڴ���һ�ֽڶ��� �����д����4�ֽڵĶ����  
typedef struct ServiceDescriptorEntry {//����ṹ����Ϊ�˹��������������� �ں�api���ڵ����� ��������ṹ�� �����ssdt  
	unsigned int *ServiceTableBase;//����ServiceTable ssdt����  
	unsigned int *ServiceCounterTableBase; //��������checked build�汾 ����  
	unsigned int NumberOfServices;//(ServiceTableBase)�������ж��ٸ�Ԫ�� �ж��ٸ���  
	unsigned char *ParamTableBase;//�������ַ ���ǲ㴫������api�Ĳ��� ռ�ö����ֽ� ���  
} ServiceDescriptorTableEntry_t, *PServiceDescriptorTableEntry_t;
#pragma pack()
__declspec(dllimport) ServiceDescriptorTableEntry_t KeServiceDescriptorTable;



//�ر�ҳֻ������
void _declspec(naked) ShutPageProtect()
{
	__asm
	{
		push eax;
		mov eax, cr0;
		and eax, ~0x10000;
		mov cr0, eax;
		pop eax;
		ret;
	}
}

//����ҳֻ������
void _declspec(naked) OpenPageProtect()
{
	__asm
	{
		push eax;
		mov eax, cr0;
		or eax, 0x10000;
		mov cr0, eax;
		pop eax;
		ret;
	}
}

//globle
ULONG_PTR g_Origin_NtOpenKey;
ULONG_PTR g_jmp_NtOpenKey;
UCHAR g_orign_funcode[5];

VOID FilterNtOpenKey()
{
	KdPrint(("%s", (char*)PsGetCurrentProcess() + 0x16c));
}

__declspec(naked)
VOID NewNtOpenKey()
{
	__asm
	{
		call FilterNtOpenKey;
		pop eax;
		mov edi, edi;
		push ebp;
		mov ebp, esp;
		jmp g_jmp_NtOpenKey;
	}

}

VOID HookNtOpenKey()
{
	ULONG_PTR u_jmp_temp;
	UCHAR jmp_code[5];
	g_Origin_NtOpenKey = KeServiceDescriptorTable.ServiceTableBase[182];
	g_jmp_NtOpenKey = g_Origin_NtOpenKey + 5;

	u_jmp_temp = (ULONG_PTR)NewNtOpenKey - g_Origin_NtOpenKey - 5;
	jmp_code[0] = 0xE8;
	*(ULONG_PTR*)&jmp_code[1] = u_jmp_temp;
	ShutPageProtect();
	RtlCopyMemory(g_orign_funcode, (PVOID)g_Origin_NtOpenKey, 5);
	RtlCopyMemory((PVOID)g_Origin_NtOpenKey, jmp_code, 5);
	OpenPageProtect();
}

VOID UnHookNtOpenKey()
{
	ShutPageProtect();
	RtlCopyMemory((PVOID)g_Origin_NtOpenKey, g_orign_funcode, 5);
	OpenPageProtect();
}

VOID DriverUnLoad(PDRIVER_OBJECT pDriverObject)
{
	UnHookNtOpenKey();
	KdPrint(("����ж�سɹ���"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	KdPrint(("�������سɹ���"));
	HookNtOpenKey();
	pDriverObject->DriverUnload = DriverUnLoad;
	return STATUS_SUCCESS;
}