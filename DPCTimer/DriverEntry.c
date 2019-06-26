#include <ntifs.h>


//global
KTIMER g_kTimer;
KDPC g_kDpc;

VOID DeferreRoutine(
	_In_ struct _KDPC *Dpc,
	_In_opt_ PVOID DeferredContext,
	_In_opt_ PVOID SystemArgument1,
	_In_opt_ PVOID SystemArgument2
)
{
	LARGE_INTEGER largeInt;
	KdPrint(("DpcTimer Called !"));
	largeInt.QuadPart = -10000000;
	KeSetTimer(&g_kTimer, largeInt, &g_kDpc);
}

VOID SetDpcTimer()
{
	LARGE_INTEGER largeInt;
	KeInitializeTimer(&g_kTimer);//��ʼ��Timer
	KeInitializeDpc(&g_kDpc, DeferreRoutine,NULL);//��ʼ��DPC
	largeInt.QuadPart = -10000000;
	KeSetTimer(&g_kTimer, largeInt, &g_kDpc);
}

VOID CancelTimer()
{
	KeCancelTimer(&g_kTimer);
}

NTSTATUS DriverUnLoad(PDRIVER_OBJECT pDriverObject)
{
	CancelTimer();
	KdPrint(("����ж�سɹ���"));
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	SetDpcTimer();
	pDriverObject->DriverUnload = DriverUnLoad;
	KdPrint(("�������سɹ���"));
	return STATUS_SUCCESS;
}