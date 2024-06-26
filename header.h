#pragma once

#include <ntddk.h>

#define DEVICE_NAME L"\\Device\\HideProcessR0"
#define SYMBOL_NAME L"\\??\\HideProcessR0"
#define IMAGE_NAME_MAX 250
#define WIN10_21H1_EPROCESS_TO_ACTIVEPROCESSLINKS_OFFSET 0x448
#define IOCTL_HIDE_BY_PID \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6666, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_IMAGE_NAME \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6667, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define kprintf(...) \
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__))

NTSTATUS CustomControl(PDEVICE_OBJECT, PIRP);
NTSTATUS CustomCreate(PDEVICE_OBJECT, PIRP);
NTKERNELAPI UCHAR* PsGetProcessImageFileName(__in PEPROCESS Process);

typedef struct SET_IMAGE_NAME {
    UCHAR bImageName[250];
    UINT64 pid;
}SET_IMAGE_NAME, *PSET_IMAGE_NAME ;