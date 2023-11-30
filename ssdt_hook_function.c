#include "ssdt_hook_struct.h"

ULONG g_uCr0;
ULONG g_Init = FALSE;

// 用NewService替换OldService, NewService为用户自定义服务函数
// 注意: OldService应该是Zw functions 以便求出服务ID
// 也可以用MmGetSystemRoutineAddress求服务ID
NTSTATUS HookService(ULONG OldService, ULONG NewService)
{
	// 如果没有初始化 提示错误
	if(!g_Init)
	{
		KdPrint(("ServiceTalbe Not Init.\n"));
		return STATUS_UNSUCCESSFUL;
	}

	WPOFF();

	// 替换钩子函数
	KdPrint(("New Service Address: %08X\n", NewService));
	*(PULONG)SERVICE_FUNCTION(OldService) = NewService;

	WPON();
	return STATUS_SUCCESS;
}

// 恢复HOOK
NTSTATUS UnHookService(ULONG OldService)
{
	if(!g_Init)
	{
		return STATUS_UNSUCCESSFUL;
	}

	WPOFF();

	// 还原钩子函数
	*(PULONG)SERVICE_FUNCTION(OldService) = OldServiceAddressTable[SERVICE_ID(OldService)];

	WPON();

	return STATUS_SUCCESS;
}

// 保存所有服务函数地址
// 在HOOK之前 应该先调用该函数
VOID InitServicesTable()
{
	ULONG i;

	// 初始化时定义该标签
	g_Init = TRUE;
	
	for(i = 0; i < KeServiceDescriptorTable->ntoskrnl.NumberOfService; i++)
	{
		OldServiceAddressTable[i] = *(PULONG)((ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTableBase + 4*i);
		KdPrint(("0x%04X : %08X\n", i, OldServiceAddressTable[i]));
	}
}

// 去除内存写保护
VOID WPOFF()
{
    ULONG uAttr;
    _asm
    {
        push eax
        mov eax, cr0
        mov uAttr, eax
        and eax, 0FFFEFFFFh
        mov cr0, eax
        pop eax
        cli
    }
    g_uCr0 = uAttr;
}

// 恢复内存写保护
VOID WPON()
{
    _asm
    {
        sti
        push eax
        mov eax, g_uCr0
        mov cr0, eax
        pop eax
    }
}