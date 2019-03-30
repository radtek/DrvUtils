#ifndef FORWARD_PROGRESS_H
#define FORWARD_PROGRESS_H

#define MAX_RESERVED_REQUESTS 8
#define MEMORY_SIZE           0x100

#include <ntddk.h>
#include <initguid.h>
#include <ntdddisk.h>

#include <wdf.h>

typedef struct {
	WDFMEMORY Memory;
} FWD_PROGRESS_REQUEST_CONTEXT, *PFWD_PROGRESS_REQUEST_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FWD_PROGRESS_REQUEST_CONTEXT, GetForwardProgressRequestContext)

NTSTATUS SetForwardProgressOnQueue(__in WDFQUEUE queue);

#endif /* FORWARD_PROGRESS_H */
