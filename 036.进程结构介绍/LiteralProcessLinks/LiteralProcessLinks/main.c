#include<ntifs.h>

EXTERN_C NTSTATUS SeLocateProcessImageName(
	__in PEPROCESS Process,
	__deref_out PUNICODE_STRING* pImageFileName
);

VOID Unload(PDRIVER_OBJECT pDriver)
{

}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	PUNICODE_STRING ImageName = NULL;
	NTSTATUS status = 0;
	PEPROCESS SysProcess = PsInitialSystemProcess;
	//PEPROCESS SysProcess = PsGetCurrentProcess();
	PEPROCESS CurProcess = SysProcess;

	ULONG_PTR ActiveProcessLinksOffset = 0xb8;


	do
	{
		if (PsGetProcessExitStatus(CurProcess) == STATUS_PENDING)
		{
			status = SeLocateProcessImageName(CurProcess, &ImageName);
			if (NT_SUCCESS(status))
			{
				DbgPrintEx(77, 0, "%wZ\r\n", ImageName);
				ExFreePool(ImageName);
			}
		}
		CurProcess = (PEPROCESS)(*(PULONG_PTR)((ULONG_PTR)CurProcess + ActiveProcessLinksOffset) - ActiveProcessLinksOffset);
	} while (SysProcess != CurProcess);



	pDriver->DriverUnload = Unload;
	return STATUS_SUCCESS;
}