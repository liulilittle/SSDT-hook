#include "ssdt_hook_struct.h"

ULONG g_uCr0;
ULONG g_Init = FALSE;

// ��NewService�滻OldService, NewServiceΪ�û��Զ��������
// ע��: OldServiceӦ����Zw functions �Ա��������ID
// Ҳ������MmGetSystemRoutineAddress�����ID
NTSTATUS HookService(ULONG OldService, ULONG NewService)
{
	// ���û�г�ʼ�� ��ʾ����
	if(!g_Init)
	{
		KdPrint(("ServiceTalbe Not Init.\n"));
		return STATUS_UNSUCCESSFUL;
	}

	WPOFF();

	// �滻���Ӻ���
	KdPrint(("New Service Address: %08X\n", NewService));
	*(PULONG)SERVICE_FUNCTION(OldService) = NewService;

	WPON();
	return STATUS_SUCCESS;
}

// �ָ�HOOK
NTSTATUS UnHookService(ULONG OldService)
{
	if(!g_Init)
	{
		return STATUS_UNSUCCESSFUL;
	}

	WPOFF();

	// ��ԭ���Ӻ���
	*(PULONG)SERVICE_FUNCTION(OldService) = OldServiceAddressTable[SERVICE_ID(OldService)];

	WPON();

	return STATUS_SUCCESS;
}

// �������з�������ַ
// ��HOOK֮ǰ Ӧ���ȵ��øú���
VOID InitServicesTable()
{
	ULONG i;

	// ��ʼ��ʱ����ñ�ǩ
	g_Init = TRUE;
	
	for(i = 0; i < KeServiceDescriptorTable->ntoskrnl.NumberOfService; i++)
	{
		OldServiceAddressTable[i] = *(PULONG)((ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTableBase + 4*i);
		KdPrint(("0x%04X : %08X\n", i, OldServiceAddressTable[i]));
	}
}

// ȥ���ڴ�д����
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

// �ָ��ڴ�д����
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