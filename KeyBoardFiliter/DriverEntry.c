#include "Common.h"

NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS		Status;
	PDEVICE_OBJECT	MyDevice;
	UNICODE_STRING	usDeviceName;
	UNICODE_STRING	usSymName;
	RtlInitUnicodeString(&usDeviceName, DEV_NAME);
	//�����豸
	Status = IoCreateDevice(pDriverObject, 0, &usDeviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &MyDevice);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("IoCreateDevice Failed !"));
		return Status;
	}
	MyDevice->Flags |= DO_BUFFERED_IO;
	RtlInitUnicodeString(&usSymName, SYM_LINK_NAME);
	//����SymbolicLink
	Status = IoCreateSymbolicLink(&usSymName, &usDeviceName);
	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(MyDevice);
		KdPrint(("IoCreateSymbolicLink Failed !"));
		return Status;
	}
	return Status;
}


NTSTATUS KillProcess(HANDLE Pid)
{
	NTSTATUS Status = STATUS_SUCCESS;
	HANDLE ProcessHandle;
	CLIENT_ID Client_Id;
	OBJECT_ATTRIBUTES ObjAttr;
	InitializeObjectAttributes(&ObjAttr, 0, 0, 0, 0);
	Client_Id.UniqueProcess = Pid;
	Client_Id.UniqueThread = 0;
	Status = ZwOpenProcess(&ProcessHandle, PROCESS_ALL_ACCESS, &ObjAttr, &Client_Id);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("ZwOpenProcess Failed !"));
		return Status;
	}
	ZwTerminateProcess(ProcessHandle, Status);
	ZwClose(ProcessHandle);
	return Status;
}


NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT pDriverObj, PIRP pIrp)
{
	NTSTATUS Status;
	Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG_PTR InLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG_PTR OutLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG_PTR IoControlCode = IrpStack->Parameters.DeviceIoControl.IoControlCode;
	PVOID pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	ULONG_PTR info;
	switch (IoControlCode)
	{
	case IOCTL_READ:
	{
		KdPrint(("IOCTL_READ"));
		memset(pIoBuffer, 0xAA, OutLength);
		info = OutLength;
	}
	break;
	case IOCTL_WRITE:
	{
		KdPrint(("IOCTL_WRITE"));
		for (size_t i = 0; i < InLength; i++)
		{
			KdPrint(("%X", ((PUCHAR)(pIoBuffer))[i]));
		}
		info = OutLength;
	}
	break;
	case IOCTL_GETPID:
	{
		KdPrint(("IOCTL_GETPID"));
		HANDLE Pid;
		Pid = *(HANDLE*)pIoBuffer;
		KdPrint(("Pid: %d", Pid));
		Status = KillProcess(Pid);
		if (NT_SUCCESS(Status))
		{
			KdPrint(("�رս��̳ɹ���"));
		}
		info = OutLength;
	}
	break;
	default:
		KdPrint(("CODE ERROR !"));
		Status = STATUS_UNSUCCESSFUL;
		break;
	}

	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = info;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return Status;
}


NTSTATUS DispatchFunction(PDEVICE_OBJECT pDriverObj, PIRP pIrp)
{
	NTSTATUS Status;
	Status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS DispatchRead(PDEVICE_OBJECT pDriverObj, PIRP pIrp)
{
	NTSTATUS Status;
	Status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS DispatchPower(PDEVICE_OBJECT pDriverObj, PIRP pIrp)
{
	NTSTATUS Status;
	Status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS DispatchPnp(PDEVICE_OBJECT pDriverObj, PIRP pIrp)
{
	NTSTATUS Status;
	Status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


VOID SetDispatchFunction(PDRIVER_OBJECT pDriverObject)
{
	for (size_t i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DispatchFunction;
	}
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
	pDriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;
	pDriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
}


NTSTATUS BIndDevice(PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS			Status;
	UNICODE_STRING		KbdName;
	//Ҫ�򿪵���������
	PDRIVER_OBJECT		KbdDriverObject;
	//����������豸
	PDEVICE_OBJECT		KbdDeviceObject;
	//˭����չ
	PDEVICE_EXTENSION	pDevExt;
	PDEVICE_OBJECT		pDevObj;
	RtlInitUnicodeString(&KbdName, L"\\Driver\\Kbdclass");
	Status = ObReferenceObjectByName(&KbdName,
		OBJ_CASE_INSENSITIVE, 
		NULL, 
		0, 
		*IoDriverObjectType, 
		KernelMode, 
		NULL, 
		(PVOID*)&KbdDriverObject);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("ObReferenceObjectByName Failed !  %X",Status));
		return Status;
	}
	else
	{
		ObDereferenceObject(KbdDriverObject);
	}
	KbdDeviceObject = KbdDriverObject->DeviceObject;
	while (KbdDeviceObject!=NULL)
	{
		Status = IoCreateDevice(pDriverObject, 
			sizeof(PDEVICE_OBJECT), 
			NULL, 
			KbdDeviceObject->DeviceType,
			KbdDeviceObject->Characteristics,
			FALSE,
			&pDevObj);
		if (!NT_SUCCESS(Status))
		{
			KdPrint(("IoCreateDevice Failed !"));
			return Status;
		}

		//�洢�豸����
		pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
		memset(pDevExt, 0, sizeof(DEVICE_EXTENSION));
		pDevExt->pDevice = pDevObj;
		pDevExt->poldDevice = KbdDeviceObject;
		//��־λ
		pDevObj->Flags |= KbdDeviceObject->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE);
		//�ñ�ʶָʾI/O��������ȫ�����͵������豸�����Open������а�ȫ��y
		pDevObj->Characteristics = KbdDeviceObject->Characteristics;
		//���豸
		PDEVICE_OBJECT topDev = IoAttachDeviceToDeviceStack(pDevObj, KbdDeviceObject);
		if (topDev == NULL)
		{
			IoDeleteDevice(pDevObj);
			KdPrint(("IoAttachDeviceToDeviceStack Failed !"));
			Status = STATUS_UNSUCCESSFUL;
			return Status;
		}
		//���󶨵��豸��ԭʼ�豸�����豸��չ��
		pDevExt->poldDevice = KbdDeviceObject;
		pDevExt->pbindDevice = topDev;
		KbdDeviceObject = KbdDeviceObject->NextDevice;
		pDevObj->Flags = pDevObj->Flags & ~DO_DEVICE_INITIALIZING;
	}
	KdPrint(("�󶨳ɹ���"));
	return Status;
}

// flags for keyboard status
#define	S_SHIFT				1
#define	S_CAPS				2
#define	S_NUM				4
static int kb_status = S_NUM;
void __stdcall print_keystroke(UCHAR sch)
{
	UCHAR	ch = 0;
	int		off = 0;

	if ((sch & 0x80) == 0)	//make
	{
		if ((sch < 0x47) ||
			((sch >= 0x47 && sch < 0x54) && (kb_status & S_NUM))) // Num Lock
		{
			ch = asciiTbl[off + sch];
		}

		switch (sch)
		{
		case 0x3A:
			kb_status ^= S_CAPS;
			break;

		case 0x2A:
		case 0x36:
			kb_status |= S_SHIFT;
			break;

		case 0x45:
			kb_status ^= S_NUM;
		}
	}
	else		//break
	{
		if (sch == 0xAA || sch == 0xB6)
			kb_status &= ~S_SHIFT;
	}

	if (ch >= 0x20 && ch < 0x7F)
	{
		DbgPrint("%C /n", ch);
	}

}


NTSTATUS CallBackKbdFilter(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	PIO_STACK_LOCATION currentIrp;
	PKEYBOARD_INPUT_DATA keyData;

	currentIrp = IoGetCurrentIrpStackLocation(Irp);

	if (NT_SUCCESS(Irp->IoStatus.Status))
	{
		keyData = (PKEYBOARD_INPUT_DATA)Irp->AssociatedIrp.SystemBuffer;

		//DbgPrint("ɨ���룺%x",keyData->MakeCode);
		DbgPrint("���� ��%s", keyData->Flags ? "����" : "����");
		print_keystroke((UCHAR)keyData->MakeCode);
	}
	if (Irp->PendingReturned)
	{
		IoMarkIrpPending(Irp);
	}
	return Irp->IoStatus.Status;
}

//��ǲ����
NTSTATUS FilterDispatchRoutin(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	PIO_STACK_LOCATION currentIrpStack;
	PDEVICE_EXTENSION pDevExt;
	//�õ��豸��չ
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	//�õ���ǰirp��
	currentIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	//����ǰirp�������²��豸irp��ջ
	IoCopyCurrentIrpStackLocationToNext(pIrp);
	//����ԭ����irp
	//pDevExt->tagIrp=pIrp;
	//����irp
	pDevExt->proxyIrp = pIrp;
	//���õ�irp���ʱ�Ļص�����
	IoSetCompletionRoutine(pDevExt->proxyIrp, CallBackKbdFilter, pDevObj, TRUE, TRUE, TRUE);
	DbgPrint("irp�ص������������.../n");
	return IoCallDriver(pDevExt->poldDevice, pDevExt->proxyIrp);
}


BOOLEAN CancelIrp(PIRP pIrp)
{
	if (pIrp == NULL)
	{
		DbgPrint("ȡ��irp����.../n");
		return FALSE;
	}
	if (pIrp->Cancel || pIrp->CancelRoutine == NULL)
	{
		DbgPrint("ȡ��irp����.../n");
		return FALSE;
	}
	if (FALSE == IoCancelIrp(pIrp))
	{
		DbgPrint("IoCancelIrp to irp����.../n");
		return FALSE;
	}
	//ȡ�����������Ϊ��
	IoSetCancelRoutine(pIrp, NULL);
	return TRUE;
}

//ж������
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	//�õ��豸
	PDEVICE_OBJECT pDevObj = pDriverObject->DeviceObject;
	while (pDevObj != NULL)
	{
		//�豸��չ
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
		PDEVICE_OBJECT pTagObj = pDevExt->pbindDevice;
		//�����
		if (pDevExt->pbindDevice != NULL)
		{
			IoDetachDevice(pDevExt->pbindDevice);
		}
		//ɾ���豸
		if (pDevExt->pDevice != NULL)
		{
			IoDeleteDevice(pDevExt->pDevice);
		}
		if (pDevExt->proxyIrp != NULL)
		{
			if (CancelIrp(pDevExt->proxyIrp))
			{
				DbgPrint("ȡ���ɹ�����</p><p>��</p><p>/n");
			}
			else
			{
				DbgPrint("ȡ��ʧ�ܡ���</p><p>��/n");
			}
		}
		//��һ���豸
		pDevObj = pDevObj->NextDevice;
	}
	KdPrint(("����ж�سɹ���"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	NTSTATUS Status;
	KdPrint(("�������سɹ���"));
	//������ǲ����
	//SetDispatchFunction(pDriverObject);
	pDriverObject->MajorFunction[IRP_MJ_READ] = FilterDispatchRoutin;
	//�����豸����
	//Status = CreateDevice(pDriverObject);
	//if (!NT_SUCCESS(Status))
	//{
	//	KdPrint(("����deviceʧ�ܣ�"));
	//}
	//else
	//{
	//	KdPrint(("����device�ɹ���"));
	//	KdPrint(("%wZ", pRegistryPath));
	//}
	//����ж������
	Status = BIndDevice(pDriverObject);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("��ʧ�ܣ�"));
	}
	else
	{
		KdPrint(("�󶨳ɹ���"));
	}
	pDriverObject->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}