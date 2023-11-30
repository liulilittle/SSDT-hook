/*
	ssdt_hook_struct.h
	包含SSDT HOOK常用的结构类型以及一些函数的定义
*/

#ifndef _SSDT_HOOK_STUCT_H_
#define _SSDT_HOOK_STUCT_H_

#include <ntddk.h>

typedef struct _SYSTEM_SERVICE_TABLE
{
	PVOID   ServiceTableBase;			// SSDT (System Service Dispatch Table)的基地址
	PULONG  ServiceCounterTableBase;	// 用于checked builds, 包含SSDT中每个服务被调用的次数
	ULONG   NumberOfService;			// 服务函数的个数, NumberOfService*4 就是整个地址表的大小
	ULONG   ParamTableBase;				// SSPT (System Service Parameter Table)的基地址
} SYSTEM_SERVICE_TABLE, *PSYSTEM_SERVICE_TABLE; 

typedef struct _SERVICE_DESCRIPTOR_TABLE
{
	SYSTEM_SERVICE_TABLE   ntoskrnl;	// ntoskrnl.exe的服务函数
	SYSTEM_SERVICE_TABLE   win32k;		// win32k.sys的服务函数,(gdi.dll/user.dll的内核支持)
	SYSTEM_SERVICE_TABLE   NotUsed1;
	SYSTEM_SERVICE_TABLE   NotUsed2;
} SYSTEM_DESCRIPTOR_TABLE, *PSYSTEM_DESCRIPTOR_TABLE;

// 
extern PSYSTEM_DESCRIPTOR_TABLE    KeServiceDescriptorTable;
/*
	lkd> ? KeServiceDescriptorTable
	Evaluate expression: -2141862144 = 8055c700
	lkd> dd 8055c700
	8055c700  8591bb08 00000000 0000013d 8594f260	<--> ntoskrnl
	            |        |        |        |
				|        |        |        +------------ ParamTableBase
				|        |        +--------------------- NumberOfService
				|        +------------------------------ ServiceCounterTableBase
				+--------------------------------------- ServiceTableBase
	8055c710  00000000 00000000 00000000 00000000
	8055c720  00000000 00000000 00000000 00000000
	8055c730  00000000 00000000 00000000 00000000
*/

// 根据 Zw_function 获取服务ID
#define SERVICE_ID(_function)			(*(PULONG)((PUCHAR)_function + 1))

// 根据 Zw_function 获取 Nt_function的地址
#define SERVICE_FUNCTION(_function)		\
		((ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTableBase + 4*SERVICE_ID(_function))
/*
	lkd> u ZwOpenProcess
	nt!ZwOpenProcess:
	80501694 b87a000000      mov     eax,7Ah  ---> ZwOpenProcess+1 = ID
*/

// ------------------------------------------------------
NTSTATUS HookService(ULONG OldService, ULONG NewService);
NTSTATUS UnHookService(ULONG OldService);
VOID InitServicesTable();
VOID WPON();
VOID WPOFF();

// ------------------------------------------------------
ULONG OldServiceAddressTable[1024];			// 保存旧的服务函数地址
//ULONG NewServiceAddressTable[1024];			// 用户自定义的服务函数地址

#endif