/*
	hook_sample.c
	�������������ʾ��
	��һ������HOOK��ZwSetInformationFile����test.txt�ļ�����ɾ��
	�ڶ�������HOOK��NtOpenProcess����PID����1000�Ľ��̲�������
*/

#include "ssdt_hook_struct.h"

// ����HOOK�ĺ���ԭ��
typedef
NTSTATUS
(__stdcall *ZWSETINFORMATIONFILE)(IN HANDLE FileHandle,
								  OUT PIO_STATUS_BLOCK IoStatusBlock,
								  IN PVOID FileInformation,
								  IN ULONG Length,
								  IN FILE_INFORMATION_CLASS FileInformationClass);
typedef
NTSTATUS
(__stdcall *NTOPENPROCESS)( OUT PHANDLE ProcessHandle,
							IN ACCESS_MASK AccessMask,                                           
							IN POBJECT_ATTRIBUTES ObjectAttributes,                                           
							IN PCLIENT_ID ClientId);

// ����ntddk.h��δ����ĺ���
// ���Ը���<<Undocument>>һ���������������
NTSYSAPI
NTSTATUS
NTAPI 
ZwOpenProcess( OUT PHANDLE ProcessHandle,
						IN ACCESS_MASK AccessMask,                                           
						IN POBJECT_ATTRIBUTES ObjectAttributes,                                           
						IN PCLIENT_ID ClientId);

// ==============================================================
// �û��Զ���HOOK����
NTSTATUS MyZwSetInformationFile(IN HANDLE FileHandle,
								OUT PIO_STATUS_BLOCK IoStatusBlock,
								IN PVOID FileInformation,
								IN ULONG Length,
								IN FILE_INFORMATION_CLASS FileInformationClass)
{
	PFILE_OBJECT pFileObject;

	// ��OldServiceAddressTable��ȡ��ԭ��������ַ
	ZWSETINFORMATIONFILE OldZwSetInformationFile = 
		(ZWSETINFORMATIONFILE)OldServiceAddressTable[SERVICE_ID(ZwSetInformationFile)];
	
	NTSTATUS ret = ObReferenceObjectByHandle(FileHandle, 
											 GENERIC_READ,
											 *IoFileObjectType, 
											 KernelMode, 
											 (PVOID*)&pFileObject, 
											 0); 
	if(NT_SUCCESS(ret))
	{
		KdPrint(("%S opened.\n", pFileObject->FileName.Buffer));
		if (wcsstr(pFileObject->FileName.Buffer, L"test.txt"))
		{
			KdPrint(("test.txt opened. Deny it.\n"));
			return STATUS_ACCESS_DENIED;
		}
	}
	
	// ����ԭ������
	return OldZwSetInformationFile( FileHandle, IoStatusBlock, FileInformation, 
								    Length, FileInformationClass);
}

NTSTATUS MyNtOpenProcess(OUT PHANDLE ProcessHandle,
						 IN ACCESS_MASK DesiredAccess,
						 IN POBJECT_ATTRIBUTES ObjectAttributes,
						 IN PCLIENT_ID ClientId )
{
	NTSTATUS rc;
	ULONG PID;
	
	NTOPENPROCESS OldNtOpenProcess = 
		(NTOPENPROCESS)OldServiceAddressTable[SERVICE_ID(ZwOpenProcess)];

	if(ClientId != NULL)
	{
		PID = (ULONG)ClientId->UniqueProcess;
        if(PID > 1000)
        {
			return STATUS_ACCESS_DENIED;
        }
	}
	return OldNtOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
}

// Unload���� ж�ع���
VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
	KdPrint(("Unload Routine.\n"));
	UnHookService((ULONG)ZwSetInformationFile);
	UnHookService((ULONG)ZwOpenProcess);
}

// DriverEntry���� ��ʼ������װ����
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
					 IN PUNICODE_STRING RegistryPath)
{
	DriverObject->DriverUnload = Unload;
	InitServicesTable();
	HookService((ULONG)ZwSetInformationFile, (ULONG)MyZwSetInformationFile);
	HookService((ULONG)ZwOpenProcess, (ULONG)MyNtOpenProcess);

	return STATUS_SUCCESS;
}