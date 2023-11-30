/*
	ssdt_hook_struct.h
	����SSDT HOOK���õĽṹ�����Լ�һЩ�����Ķ���
*/

#ifndef _SSDT_HOOK_STUCT_H_
#define _SSDT_HOOK_STUCT_H_

#include <ntddk.h>

typedef struct _SYSTEM_SERVICE_TABLE
{
	PVOID   ServiceTableBase;			// SSDT (System Service Dispatch Table)�Ļ���ַ
	PULONG  ServiceCounterTableBase;	// ����checked builds, ����SSDT��ÿ�����񱻵��õĴ���
	ULONG   NumberOfService;			// �������ĸ���, NumberOfService*4 ����������ַ��Ĵ�С
	ULONG   ParamTableBase;				// SSPT (System Service Parameter Table)�Ļ���ַ
} SYSTEM_SERVICE_TABLE, *PSYSTEM_SERVICE_TABLE; 

typedef struct _SERVICE_DESCRIPTOR_TABLE
{
	SYSTEM_SERVICE_TABLE   ntoskrnl;	// ntoskrnl.exe�ķ�����
	SYSTEM_SERVICE_TABLE   win32k;		// win32k.sys�ķ�����,(gdi.dll/user.dll���ں�֧��)
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

// ���� Zw_function ��ȡ����ID
#define SERVICE_ID(_function)			(*(PULONG)((PUCHAR)_function + 1))

// ���� Zw_function ��ȡ Nt_function�ĵ�ַ
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
ULONG OldServiceAddressTable[1024];			// ����ɵķ�������ַ
//ULONG NewServiceAddressTable[1024];			// �û��Զ���ķ�������ַ

#endif