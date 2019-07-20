﻿
#include "pch.h"
#include <iostream>
#include <windows.h>
#include <winioctl.h>


#define IOCTL_BASE 0x800
#define IOCTL_CODE(i) CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_BASE+i,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_READ IOCTL_CODE(1)
#define IOCTL_WRITE IOCTL_CODE(2)
#define IOCTL_GETPID IOCTL_CODE(3)


typedef struct WRIO
{
	ULONG_PTR	Address;
	BOOLEAN		IsRead;
	size_t		Length;
	BOOLEAN		Status;
}WRIO, *PWRIO;


//template <typename T>
//BOOLEAN ReadMemory(ULONG_PTR Address,T& Buffer)
//{
//	WRIO IO;
//	IO.Address = Address;
//	IO.IsRead = TRUE;
//	IO.Length = sizeof(Buffer);
//	DeviceIoControl(hDevice, IOCTL_READ, &IO, sizeof(WRIO), Buffer, IO.Length, &dwRet, NULL);
//	return Buffer.Status;
//}

int main()
{
	DWORD dwRet = 0;
	UCHAR InputBuffer[1024] = {0};
	UCHAR OutputBuffer[1024] = {0};
	BOOL bRet;
	int times;
	while (true)
	{ 
		//7FF73D3C0000
		((PWRIO)InputBuffer)->IsRead = TRUE;
		printf("请输入要读取的地址 ：");
		scanf_s("%I64X", &((PWRIO)InputBuffer)->Address);
		printf("请输入要读取的大小 ：");
		scanf_s("%X", &((PWRIO)InputBuffer)->Length);
		printf("请输入要读取次数 ：");
		scanf_s("%d", &times);
		HANDLE hDevice = CreateFile(L"\\\\.\\MySymLink",
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL, 
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			printf("CreateFile Failed !");
			return 1;
		}
		//读取数据
		DWORD_PTR start = GetTickCount();
		for (size_t i = 0; i < times; i++)
		{
			//InputBuffer.Address += i*2;
			bRet = DeviceIoControl(hDevice, IOCTL_READ, InputBuffer, sizeof(WRIO), OutputBuffer, sizeof(WRIO) + ((PWRIO)InputBuffer)->Length, &dwRet, NULL);
			//printf("%X ", *(UCHAR*)OutputBuffer);
		}
		DWORD_PTR  end = GetTickCount();
		printf("GetTickCount: %d\n", end - start);
		if (bRet)
		{
			printf("OutPut buffer:%d Bytes !\n", dwRet);
			for (size_t i = sizeof(WRIO); i < dwRet; i++)
			{
				printf("字节 : %X ", OutputBuffer[i]);
			}
		}
		printf("请输入要写入的数据 ：");
		scanf_s("%X", InputBuffer+sizeof(WRIO));
		((PWRIO)InputBuffer)->IsRead = FALSE;
		//写入数据
		//bRet = DeviceIoControl(hDevice, IOCTL_WRITE, InputBuffer, sizeof(WRIO)+ ((PWRIO)InputBuffer)->Length, OutputBuffer, sizeof(WRIO), &dwRet, NULL);
		((PWRIO)InputBuffer)->IsRead = TRUE;
		bRet = DeviceIoControl(hDevice, IOCTL_READ, InputBuffer, sizeof(WRIO), OutputBuffer, sizeof(WRIO) + ((PWRIO)InputBuffer)->Length, &dwRet, NULL);
		if (bRet)
		{
			printf("OutPut buffer:%d Bytes !\n", dwRet);
			for (size_t i = sizeof(WRIO); i < dwRet; i++)
			{
				printf("字节 : %X ", OutputBuffer[i]);
			}
		}
		CloseHandle(hDevice);
	}
}


