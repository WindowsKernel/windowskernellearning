#include <ntifs.h>

#pragma pack(1)
typedef struct _GDT{
	SHORT Limit;
	UINT32 Base;
}GDT;
#pragma pack(0)
typedef GDT IDT;

char bufcode[6] = { 0 };
UINT32 int3offset = 0;

VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	DbgPrintEx(77, 0, "hook int3 unloaded\n");
}

// ���壬�޸�CS=8��Ȼ����ת��MyCode
__declspec(naked) VOID Stub()
{
	__asm
	{
		sti;
		jmp fword ptr ds:[bufcode];
	}
	
}

// ������ DbgPrint��Ȼ����ת��int3ԭ���ĵ�ַ
__declspec(naked) VOID MyCode()
{
	DbgPrintEx(77, 0, "int3 called!\n");
	__asm
	{
		cli;
		mov eax, int3offset;
		jmp eax;
	}
}


// �Զ����HOOK�������޸�int3�Ķ�ѡ���ӣ���GDT 48 д���µĴ�������������޸�base
VOID HookInt3()
{
	GDT gdtr;
	IDT idtr;
	__asm
	{
		sgdt gdtr;
		sidt idtr;
	}
	PUINT64 idt = (PUINT64)idtr.Base;
	PUINT64 gdt = (PUINT64)gdtr.Base;
	int3offset = (UINT32)(idt[3] >> 48) * 0x10000 + ((UINT32)idt[3] & 0x0000ffff);
	DbgPrintEx(77, 0, "int3 offset %x\n", int3offset);
	
	// ����08��48�����޸�base��int3ʱ������Stub
	gdt[9] = gdt[1];
	UINT32 base = (UINT32)Stub - int3offset;
	DbgPrintEx(77, 0, "int3 new base: %x == %x - %x\n", base, (UINT32)Stub, int3offset);
	((char*)(&gdt[9]))[7] = ((char*)&base)[3];
	((char*)(&gdt[9]))[4] = ((char*)&base)[2];
	((char*)(&gdt[9]))[3] = ((char*)&base)[1];
	((char*)(&gdt[9]))[2] = ((char*)&base)[0];
	DbgPrintEx(77, 0, "GDT08 %p: %016llx\n", &gdt[1], gdt[1]);
	DbgPrintEx(77, 0, "GDT48 %p: %016llx\n", &gdt[9], gdt[9]);

	// ����bufcode������ Stub ������ת��CS�Ļ�08��������ת�� MyCode
	*(UINT32*)bufcode = (UINT32)MyCode;
	bufcode[4] = 0x8;

	// �޸� int3 �Ķ�ѡ����Ϊ48
	DbgPrintEx(77, 0, "idt3: %llx\n", idt[3]);
	DbgPrintEx(77, 0, "int3 selector %x\n", ((char*)&idt[3])[2]);
	((char*)&idt[3])[2] = 0x48;
	DbgPrintEx(77, 0, "new int3 selector %x\n", ((char*)&idt[3])[2]);
}
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	DbgPrintEx(77, 0, "-------------------------------\n");
	DbgPrintEx(77, 0, "hook int3 loaded\n");
	pDriver->DriverUnload = DriverUnload;
	HookInt3();
	return STATUS_SUCCESS;
}