//---------------------------------------------------------------------------

#ifndef CanItfH
#define CanItfH
//---------------------------------------------------------------------------
// UcanIntf.h: Schnittstelle f?r die Klasse CUcanIntf.
//
//////////////////////////////////////////////////////////////////////

// allow access to functions for C++ applications as well
#ifdef __cplusplus
extern "C"
{
#endif

#include "Usbcan32.h"

#define WM_UCAN_RECEIVE             (WM_USER + 1)
#define WM_UCAN_STATUS              (WM_USER + 2)
#define WM_UCAN_CONNECT             (WM_USER + 10)
#define WM_UCAN_DISCONNECT          (WM_USER + 11)
#define WM_UCAN_FATAL_DISCONNECT    (WM_USER + 12)

typedef void (* UCanOnRead_f)(BYTE bChannel_p,tCanMsgStruct *pCanMsg_p);

BOOL UCanInit (UCanOnRead_f OnRead_p, BYTE bChannel_p, BYTE bDeviceNr_p, WORD wBTR_p, DWORD dwFromId_p, DWORD dwToId_p);
BOOL UCanShutDown (BYTE bChannel_p);
BOOL UCanWriteMsg (BYTE bChannel_p, tCanMsgStruct *pCanMsg_p, DWORD dwCount_p );
BOOL UCanReady (BYTE bChannel_p);
BOOL UCanReset (BYTE bChannel_p, DWORD dwResetFlags_p);
void UcanPoll(BYTE bChannel_p);
#ifdef __cplusplus
}
#endif

#endif
