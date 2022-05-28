#include<ntifs.h>
#include<strsafe.h>

BOOLEAN Loop = TRUE;

EXTERN_C NTSTATUS SeLocateProcessImageName(
	__in PEPROCESS Process,
	__deref_out PUNICODE_STRING* pImageFileName
);

VOID KSleep(ULONG ms, BOOLEAN Alert)
{
	LARGE_INTEGER larTime = { 0 };
	larTime.QuadPart = ((ULONG64)-10000) * ms;
	KeDelayExecutionThread(KernelMode, Alert, &larTime);
}

VOID Unload(PDRIVER_OBJECT pDriver)
{
	Loop = FALSE;
	KSleep(5000, FALSE);
}

VOID FuckDbgPort(
	_In_ PVOID StartContext
)
{
	
	NTSTATUS status = 0;
	PEPROCESS SysProcess = PsInitialSystemProcess;	
	ULONG_PTR ActiveProcessLinksOffset = 0xb8;
	UNICODE_STRING ProtectProcessName = RTL_CONSTANT_STRING(L"\\Device\\HarddiskVolume1\\Users\\win7x86\\Desktop\\KmdManager.exe");
	ULONG_PTR DebugPortOffset = 0xec;

	while (Loop)
	{
		PEPROCESS CurProcess = SysProcess;
		PUNICODE_STRING ImageName = NULL;
		do
		{
			if (PsGetProcessExitStatus(CurProcess) == STATUS_PENDING)
			{
				status = SeLocateProcessImageName(CurProcess, &ImageName);
				if (NT_SUCCESS(status))
				{
					if (0 == RtlCompareUnicodeString(ImageName, &ProtectProcessName, TRUE))
					{
						DbgPrintEx(77, 0, "保护: %wZ\r\n", ImageName);
						*(ULONG*)((ULONG_PTR)CurProcess + DebugPortOffset) = 0; // 别调了
					}
					ExFreePool(ImageName);
				}
			}
			CurProcess = (PEPROCESS)(*(PULONG_PTR)((ULONG_PTR)CurProcess + ActiveProcessLinksOffset) - ActiveProcessLinksOffset);
		} while (SysProcess != CurProcess);
		KSleep(1000, FALSE);
	}
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	HANDLE hThread = NULL;

	NTSTATUS st = PsCreateSystemThread(&hThread,THREAD_ALL_ACCESS,NULL,NULL,NULL, FuckDbgPort, pDriver);
	
	if (NT_SUCCESS(st))
	{
		NtClose(hThread);
	}

	pDriver->DriverUnload = Unload;
	return STATUS_SUCCESS;
}