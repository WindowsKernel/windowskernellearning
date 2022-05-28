#include <ntifs.h>

typedef struct _HANDLE_TABLE {
	ULONG_PTR TableCode;
	// ...
}HANDLE_TABLE,*PHANDLE_TABLE;

typedef struct _HANDLE_TABLE_ENTRY {
	ULONG Object;
	ULONG Attr;
}HANDLE_TABLE_ENTRY,* PHANDLE_TABLE_ENTRY;

PHANDLE_TABLE GetPspCidTable();
VOID LiteralHandleTable(PHANDLE_TABLE HandleTable);

// 未文档化函数
typedef POBJECT_TYPE(NTAPI* _ObGetObjectType)(__in PVOID Object);

VOID Unload(PDRIVER_OBJECT pDriver)
{

}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	PHANDLE_TABLE PspCidTable = GetPspCidTable();
	DbgPrintEx(77, 0, "%x\r\n", PspCidTable->TableCode);
	LiteralHandleTable(PspCidTable);
	
	pDriver->DriverUnload = Unload;
	return STATUS_SUCCESS;
}

// 不支持x64
PHANDLE_TABLE GetPspCidTable()
{
	PUCHAR data = (PUCHAR)PsLookupThreadByThreadId;
#ifdef _X86_
	for (int i = 0;i<100; i++)
	{
		// mov edi, [_PspCidTable]
		// call xxx
		if (memcmp(&data[i], "\x8b\x3d", 2) == 0 && data[i+6]== 0xe8)
		{
			return *(PHANDLE_TABLE*)(*(ULONG_PTR*)&data[i + 2]);
		}
	}
#else
	for (int i = 0; i < 100; i++)
	{
		// call PspReferenceCidTableEntry
		// mov rbx, rax
		if (data[i] == 0xe8 && memcmp(&data[i+5],"\x48\x8b\xd8",3) == 0)
		{
			ULONG32 E8Code = *(ULONG32*)&data[i + 1];
			ULONG_PTR _PspReferenceCidTableEntry = E8Code + (ULONG_PTR)&data[i + 5];
			for (int j = 0; j < 100; j++)
			{
				if ()
			}
		}
	}
#endif

	return 0;
}

VOID LiteralHandleTable(PHANDLE_TABLE HandleTable)
{
	UNICODE_STRING FuncName = RTL_CONSTANT_STRING(L"ObGetObjectType");
	_ObGetObjectType ObGetObjectType_I = MmGetSystemRoutineAddress(&FuncName);
	ULONG TableLevel = HandleTable->TableCode & 3;
	PUCHAR TableCode = HandleTable->TableCode & ~3;
	switch (TableLevel)
	{
	case 0:
	{
		DbgPrintEx(77,0,"一级句柄表\r\n");
		for (int i = 0; i < PAGE_SIZE; i+=8)
		{
			PHANDLE_TABLE_ENTRY entry = (PHANDLE_TABLE_ENTRY)&TableCode[i];
			if (MmIsAddressValid(entry->Object & ~7))
			{
				POBJECT_TYPE ObjTypetype = ObGetObjectType_I(entry->Object & ~7);
				if (ObjTypetype == *PsProcessType)
				{
					DbgPrintEx(77, 0, "进程对象\r\n");
				}
				else if (ObjTypetype == *PsThreadType)
				{
					DbgPrintEx(77, 0, "线程对象\r\n");
				}
				else
				{
					DbgPrintEx(77, 0, "其他类型对象\r\n");
				}
			}
		}
		break;
	}
	case 1:
	{
		DbgPrintEx(77, 0, "二级句柄表\r\n");
		for (int j = 0;j < PAGE_SIZE; j+=sizeof(ULONG_PTR))
		{
			PUCHAR Level2HandleTable = (PUCHAR)(*(PULONG_PTR)&TableCode[j]);
			if (MmIsAddressValid(Level2HandleTable))
			{
				for (int i = 0; i < PAGE_SIZE; i += 8)
				{
					PHANDLE_TABLE_ENTRY entry = (PHANDLE_TABLE_ENTRY)&Level2HandleTable[i];
					if (MmIsAddressValid(entry->Object & ~7))
					{
						POBJECT_TYPE ObjTypetype = ObGetObjectType_I(entry->Object & ~7);
						if (ObjTypetype == *PsProcessType)
						{
							DbgPrintEx(77, 0, "进程对象\r\n");
						}
						else if (ObjTypetype == *PsThreadType)
						{
							DbgPrintEx(77, 0, "线程对象\r\n");
						}
						else
						{
							DbgPrintEx(77, 0, "其他类型对象\r\n");
						}
					}
				}
			}
		}
		break;
	}
	case 2:
	{
		DbgPrintEx(77, 0, "三级句柄表\r\n");
		
		break;
	}
	}
}