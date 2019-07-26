#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include "resource.h"

#define IOCTL_BASE 0x800
#define IOCTL_CODE(i) CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_BASE+i,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_READ IOCTL_CODE(1)//��
#define IOCTL_WRITE IOCTL_CODE(2)//д
#define IOCTL_INIT IOCTL_CODE(3)//��ʼ��
#define IOCTL_GMODHAN IOCTL_CODE(4)//ȡģ���ַ
#define IOCTL_FORCEDELETE IOCTL_CODE(5)//ǿɾ�ļ�

typedef struct WRIO
{
	LPCVOID		VirtualAddress;
	size_t		Length;
} WRIO, *PWRIO;

class XDriverIO
{
private:
	//���豸����
	BOOLEAN InitDevice(ULONG_PTR Pid);
	//�ر��豸����
	VOID CloseDevice(HANDLE hDevice);
	//���ý���
	VOID SetPid();
	//���ֽڼ�
	VOID ReadRaw(LPCVOID lpAddress, LPVOID lpBuffer, SIZE_T nSize);
	//д�ֽڼ�
	BOOLEAN WriteRaw(LPCVOID lpAddress, LPVOID lpBuffer, SIZE_T nSize);
	//�ж��Ƿ�Ϊ����ԱȨ��
	bool isElevated();

	//��������
	BOOL LoadDriver(std::wstring TargetDriver, std::wstring TargetServiceName, std::wstring TargetServiceDesc);
	//ж������
	BOOL StopService(std::wstring TargetServiceName);
	//ȡ��ǰ·��
	std::wstring ExePath();

public:
	XDriverIO(ULONG_PTR Pid);
	~XDriverIO();
	//���ڴ�
	template <typename T>
	T ReadMemory(ULONG_PTR Address);
	//д�ڴ�
	template <typename T>
	BOOLEAN WriteMemory(ULONG_PTR Address, T Value);
	//���ַ���
	template<class CharT>
	std::basic_string<CharT> ReadString(DWORD_PTR address, size_t max_length);
	//ȡģ���ַ
	HANDLE GetMoudleHandle(LPCWSTR MoudleName);
	//ǿɾ�ļ�
	BOOLEAN ForceDelete(LPCWSTR FilePath);


private:
	//���뻺����
	LPVOID  m_InBuffer;
	//���������
	LPVOID  m_OutBuffer;
	HANDLE	m_hDevice;
	ULONG_PTR m_Pid = 0;
	const LPCWSTR SymLink = L"\\\\.\\MySymLink";
};

template<typename T>
inline T XDriverIO::ReadMemory(ULONG_PTR Address)
{
	this->ReadRaw((LPCVOID)Address, m_OutBuffer, sizeof(T));
	return *(T*)m_OutBuffer;
}

template<typename T>
inline BOOLEAN XDriverIO::WriteMemory(ULONG_PTR Address, T Value)
{
	return this->WriteRaw((LPCVOID)Address, &Value, sizeof(T));
}

template<class CharT>
inline std::basic_string<CharT> XDriverIO::ReadString(DWORD_PTR address, size_t max_length)
{
	std::basic_string<CharT> str(max_length, CharT());
	this->ReadRaw((LPVOID)address, &str[0], sizeof(CharT) * max_length);
	auto it = str.find(CharT());
	if (it == str.npos) str.clear();
	else str.resize(it);
	return str;
}
