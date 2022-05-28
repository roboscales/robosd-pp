//---------------------------------------------------------------------------


#pragma hdrstop

#include "CanItf.h"

//---------------------------------------------------------------------------

//#pragma package(smart_init)

void PUBLIC UcanCallback (tUcanHandle UcanHandle_p, DWORD dwEvent_p, BYTE bChannel_p, void* pArg_p);
void PUBLIC UcanConnectControl (DWORD dwEvent_p, DWORD dwParam_p, void* pArg_p);


void ASSERT(byte x){
         //if(!x) errlog(_LP,"assert %d",GetLastError());
//			 printf( "assert %d", GetLastError());
}

tUcanHandle m_UcanHandle = USBCAN_INVALID_HANDLE;
UCanOnRead_f mOnRead;
BOOL UCanInit (UCanOnRead_f OnRead_p,BYTE bChannel_p, BYTE bDeviceNr_p, WORD wBTR_p, DWORD dwFromId_p, DWORD dwToId_p)
{

    UCANRET bRet              = USBCAN_SUCCESSFUL;
    tUcanInitCanParam   InitParam;
    DWORD               dwVersion;
    mOnRead = OnRead_p;

    UcanInitHwConnectControlEx (UcanConnectControl, 0);

    // get versions of driver files
    dwVersion = UcanGetVersionEx (kVerTypeUserDll);   // read version of USBCAN32.DLL
    dwVersion = UcanGetVersionEx (kVerTypeSysDrv);    // read version of USBCAN.SYS
    dwVersion = UcanGetVersionEx (kVerTypeSysL4);     // read version of USBCANL4.SYS (loader for USB-CAnmodul1)
    dwVersion = UcanGetVersionEx (kVerTypeCpl);       // read version of USBCANCP.CPL

   // initialize hardware
    bRet = UcanInitHardwareEx (&m_UcanHandle, bDeviceNr_p, UcanCallback, 0);
    if (bRet != USBCAN_SUCCESSFUL)
	{
		UcanDeinitHwConnectControl();
//		printf("UcanInitHardwareEx : %d", bRet);
        //errlog(_LP,"UcanInitHardwareEx : %d",bRet);
        return FALSE;
    }


    // fill out initialisation struct
    memset (&InitParam, 0, sizeof (InitParam));
    InitParam.m_dwSize                  = sizeof (InitParam);           // size of this struct
    InitParam.m_bMode                   = kUcanModeTxEcho;              // normal operation mode
    InitParam.m_bBTR0                   = HIBYTE (wBTR_p);              // baudrate
    InitParam.m_bBTR1                   = LOBYTE (wBTR_p);
    InitParam.m_bOCR                    = USBCAN_OCR_DEFAULT;           // standard output
    InitParam.m_dwAMR                   = USBCAN_CALCULATE_AMR (FALSE, dwFromId_p, dwToId_p, FALSE, TRUE);
    InitParam.m_dwACR                   = USBCAN_CALCULATE_ACR (FALSE, dwFromId_p, dwToId_p, FALSE, TRUE);
    InitParam.m_dwBaudrate              = USBCAN_BAUDEX_USE_BTR01;
    InitParam.m_wNrOfRxBufferEntries    = USBCAN_DEFAULT_BUFFER_ENTRIES;
    InitParam.m_wNrOfTxBufferEntries    = USBCAN_DEFAULT_BUFFER_ENTRIES;

    // initialize CAN channel 0
     bRet = UcanInitCanEx2 (m_UcanHandle, 0, &InitParam);
     if (bRet != USBCAN_SUCCESSFUL)
     {
//		 printf("UcanInitCanEx2 : %d", bRet);
        //errlog(_LP,"UcanInitCanEx2 : %d",bRet);
        UcanDeinitCanEx (m_UcanHandle, bChannel_p);
        return FALSE;
     }
	 UcanSetTxTimeout(m_UcanHandle, bChannel_p ,1);

    return TRUE;

}


BOOL UCanShutDown (BYTE bChannel_p)
{

    UcanDeinitCanEx (m_UcanHandle, bChannel_p);
    UcanDeinitHardware (m_UcanHandle);
	UcanDeinitHwConnectControl();
	m_UcanHandle = USBCAN_INVALID_HANDLE;
    return TRUE;

}


BOOL UCanWriteMsg (BYTE bChannel_p, tCanMsgStruct *pCanMsg_p, DWORD dwCount_p)
{

BOOL    fRet    = FALSE;
UCANRET bRet;
DWORD   dwSavedCount;

    ASSERT (pCanMsg_p != NULL);

   // save number of CAN messages which should be sent
    dwSavedCount = dwCount_p;

    // write CAN message(s)
    bRet = UcanWriteCanMsgEx (m_UcanHandle, bChannel_p, pCanMsg_p, &dwCount_p);
   if (USBCAN_CHECK_TX_OK (bRet))
   {
        fRet = TRUE;                            // some or all CAN messages successfully sent (warnings ignored)
   }
   if (USBCAN_CHECK_ERROR (bRet))
   {
            fRet = FALSE;
   }

    return fRet;

}

BOOL UCanReset (BYTE bChannel_p, DWORD dwResetFlags_p)
{
    return UcanResetCanEx (m_UcanHandle, bChannel_p, dwResetFlags_p) == USBCAN_SUCCESSFUL;
}


BOOL UcanReadMsg (BYTE* pbChannel_p, tCanMsgStruct *pCanMsg_p, DWORD* pdwCount_p)
{

BOOL    fRet    = FALSE;
UCANRET bRet;

    ASSERT (pCanMsg_p != NULL);
    ASSERT (pbChannel_p != NULL);

    // read CAN message(s) from any channel
    *pbChannel_p = USBCAN_CHANNEL_ANY;
    bRet = UcanReadCanMsgEx (m_UcanHandle, pbChannel_p, pCanMsg_p, pdwCount_p);
    if (USBCAN_CHECK_VALID_RXCANMSG (bRet)) // (warnings ignored)
    {
        fRet = TRUE;                        // NOTE: also a warning returns a valid CAN message !
    }
    if (USBCAN_CHECK_ERROR (bRet))
    {
        fRet = FALSE;
    }

    return fRet;

}


tCanMsgStruct CanInMsg[100];

void UcanPoll(BYTE bChannel_p){
    DWORD pdwCount=100;
    BYTE pbChannel;
    tCanMsgStruct * pCanInMsg= CanInMsg;
    if ( UcanReadMsg(&pbChannel,pCanInMsg, &pdwCount) ){
        while(pdwCount>0){
            mOnRead(bChannel_p,pCanInMsg);
            pCanInMsg++;
            pdwCount--;
        }
    }
}
/*
bool UcanOnRead BYTE bChannel_p)
{
WPARAM UcanHandle_p = m.WParam;
LPARAM lParam_p = m.LParam;
BOOL            fRet;
BYTE            bChannel;
DWORD           dwRxCount, dwIndex;
tCanMsgStruct * aRxCanMsg;
        // read CAN messages until the buffer is empty
        do
        {
            // read some CAN message
            dwRxCount = GET_ENTRY_COUNT (tCanMsgStruct, aRxCanMsg_l);
            fRet = pUsbCanModul->ReadMsg (&bChannel, &aRxCanMsg_l[0], &dwRxCount);
            if (fRet != FALSE)
            {
                dwIndex = 0;
                while (dwRxCount > 0)
                {
                        aRxCanMsg = aRxCanMsg_l+dwIndex;
                        Lock->Acquire();
                        try{
                                protoDispatch(aRxCanMsg->m_dwID,aRxCanMsg->m_bData,&UpdateActuator);
                        }__finally{
                                Lock->Release();
                        }
                       dwIndex++;
                       dwRxCount--;
                }
                dReadLed->Active = True;
                ReadLedTimer->Enabled = True;
            }

        } while (fRet != FALSE);
        Application->ProcessMessages();

    }

    return 0;
}
*/

void PUBLIC UcanCallback (tUcanHandle UcanHandle_p, DWORD dwEvent_p, BYTE bChannel_p, void* pArg_p)
{

    switch (dwEvent_p)
    {
        // hardware initialized
        case USBCAN_EVENT_INITHW:
            break;

        // CAN interface initialized
        case USBCAN_EVENT_INITCAN:
            break;

        // CAN message received
        case USBCAN_EVENT_RECEIVE:

            //UcanOnRead(bChannel_p);
            break;

        // status message received
        case USBCAN_EVENT_STATUS:

            break;

        // CAN interface deinitialized
        case USBCAN_EVENT_DEINITCAN:
            break;

        // hardware deinitialized
        case USBCAN_EVENT_DEINITHW:
            break;

        // unknown event
        default:
            break;
    }

}

void PUBLIC UcanConnectControl (DWORD dwEvent_p, DWORD dwParam_p, void* pArg_p)
{

    //----------------------------------------------------------------------------
    // NOTE:
    // Do not call functions of USBCAN32.DLL directly from this callback handler.
    // Use events or windows messages to notify the event to the application.
    //----------------------------------------------------------------------------

    // check event
    switch (dwEvent_p)
    {
        // new USB-CANmodul connected
        case USBCAN_EVENT_CONNECT:

            break;

        // an other USB-CANmodul disconnected
        case USBCAN_EVENT_DISCONNECT:

            break;

        // an used USB-CANmodul disconnected
        case USBCAN_EVENT_FATALDISCON:

            break;

        // unknown event
        default:
            break;
    }

}

BOOL UCanReady(BYTE bChannel_p){
	DWORD count=123;
	DWORD ret = UcanGetMsgPending(m_UcanHandle, bChannel_p, USBCAN_PENDING_FLAG_TX_ALL, &count);
	return ret == USBCAN_SUCCESSFUL/* && count == 0*/;
}


