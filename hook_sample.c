/*
	hook_sample.c
	这里给出了两个示例
	第一个例子HOOK了ZwSetInformationFile保护test.txt文件不被删除
	第二个例子HOOK了NtOpenProcess保护PID大于1000的进程不被结束
*/

#include "ssdt_hook_struct.h"

// 定义HOOK的函数原型
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

// 对于ntddk.h中未定义的函数
// 可以根据<<Undocument>>一书在这里给出定义
NTSYSAPI
NTSTATUS
NTAPI 
ZwOpenProcess( OUT PHANDLE ProcessHandle,
						IN ACCESS_MASK AccessMask,                                           
						IN POBJECT_ATTRIBUTES ObjectAttributes,                                           
						IN PCLIENT_ID ClientId);

// ==============================================================
// 用户自定义HOOK例程
NTSTATUS MyZwSetInformationFile(IN HANDLE FileHandle,
								OUT PIO_STATUS_BLOCK IoStatusBlock,
								IN PVOID FileInformation,
								IN ULONG Length,
								IN FILE_INFORMATION_CLASS FileInformationClass)
{
	PFILE_OBJECT pFileObject;

	// 在OldServiceAddressTable中取出原服务函数地址
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
	
	// 调用原服务函数
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

// Unload例程 卸载钩子
VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
	KdPrint(("Unload Routine.\n"));
	UnHookService((ULONG)ZwSetInformationFile);
	UnHookService((ULONG)ZwOpenProcess);
}

// DriverEntry例程 初始化并安装钩子
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
					 IN PUNICODE_STRING RegistryPath)
{
	DriverObject->DriverUnload = Unload;
	InitServicesTable();
	HookService((ULONG)ZwSetInformationFile, (ULONG)MyZwSetInformationFile);
	HookService((ULONG)ZwOpenProcess, (ULONG)MyNtOpenProcess);

	return STATUS_SUCCESS;
}