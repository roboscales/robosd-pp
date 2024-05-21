

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Mon Oct 15 15:03:03 2018
 */
/* Compiler settings for mcbcom_i.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __mcbcom_i_h__
#define __mcbcom_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IMcbCommPlugin_FWD_DEFINED__
#define __IMcbCommPlugin_FWD_DEFINED__
typedef interface IMcbCommPlugin IMcbCommPlugin;
#endif 	/* __IMcbCommPlugin_FWD_DEFINED__ */


#ifndef __IMcbCommPluginInfo_FWD_DEFINED__
#define __IMcbCommPluginInfo_FWD_DEFINED__
typedef interface IMcbCommPluginInfo IMcbCommPluginInfo;
#endif 	/* __IMcbCommPluginInfo_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IMcbCommPlugin_INTERFACE_DEFINED__
#define __IMcbCommPlugin_INTERFACE_DEFINED__

/* interface IMcbCommPlugin */
/* [uuid][object] */ 


EXTERN_C const IID IID_IMcbCommPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("48A185C1-FFDB-11d3-80E3-00C04F176153")
    IMcbCommPlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Configure( 
            /* [in] */ HWND hwndParent,
            /* [string][in] */ LPCOLESTR orgString,
            /* [string][out] */ LPOLESTR *pNewString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenPort( 
            /* [string][in] */ LPCOLESTR connectString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClosePort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendRecvPacket( 
            /* [in] */ BYTE cmd,
            /* [size_is][in] */ const BYTE *cmdBuff,
            /* [in] */ WORD cmdSize,
            /* [size_is][out] */ BYTE *respBuff,
            /* [in] */ WORD respSize,
            /* [in] */ BYTE featureId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockFeature( 
            /* [in] */ BYTE featureId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnlockFeature( 
            /* [in] */ BYTE featureId) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMcbCommPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMcbCommPlugin * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMcbCommPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMcbCommPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *Configure )( 
            IMcbCommPlugin * This,
            /* [in] */ HWND hwndParent,
            /* [string][in] */ LPCOLESTR orgString,
            /* [string][out] */ LPOLESTR *pNewString);
        
        HRESULT ( STDMETHODCALLTYPE *OpenPort )( 
            IMcbCommPlugin * This,
            /* [string][in] */ LPCOLESTR connectString);
        
        HRESULT ( STDMETHODCALLTYPE *ClosePort )( 
            IMcbCommPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *SendRecvPacket )( 
            IMcbCommPlugin * This,
            /* [in] */ BYTE cmd,
            /* [size_is][in] */ const BYTE *cmdBuff,
            /* [in] */ WORD cmdSize,
            /* [size_is][out] */ BYTE *respBuff,
            /* [in] */ WORD respSize,
            /* [in] */ BYTE featureId);
        
        HRESULT ( STDMETHODCALLTYPE *LockFeature )( 
            IMcbCommPlugin * This,
            /* [in] */ BYTE featureId);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockFeature )( 
            IMcbCommPlugin * This,
            /* [in] */ BYTE featureId);
        
        END_INTERFACE
    } IMcbCommPluginVtbl;

    interface IMcbCommPlugin
    {
        CONST_VTBL struct IMcbCommPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMcbCommPlugin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IMcbCommPlugin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IMcbCommPlugin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IMcbCommPlugin_Configure(This,hwndParent,orgString,pNewString)	\
    ( (This)->lpVtbl -> Configure(This,hwndParent,orgString,pNewString) ) 

#define IMcbCommPlugin_OpenPort(This,connectString)	\
    ( (This)->lpVtbl -> OpenPort(This,connectString) ) 

#define IMcbCommPlugin_ClosePort(This)	\
    ( (This)->lpVtbl -> ClosePort(This) ) 

#define IMcbCommPlugin_SendRecvPacket(This,cmd,cmdBuff,cmdSize,respBuff,respSize,featureId)	\
    ( (This)->lpVtbl -> SendRecvPacket(This,cmd,cmdBuff,cmdSize,respBuff,respSize,featureId) ) 

#define IMcbCommPlugin_LockFeature(This,featureId)	\
    ( (This)->lpVtbl -> LockFeature(This,featureId) ) 

#define IMcbCommPlugin_UnlockFeature(This,featureId)	\
    ( (This)->lpVtbl -> UnlockFeature(This,featureId) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMcbCommPlugin_INTERFACE_DEFINED__ */


#ifndef __IMcbCommPluginInfo_INTERFACE_DEFINED__
#define __IMcbCommPluginInfo_INTERFACE_DEFINED__

/* interface IMcbCommPluginInfo */
/* [uuid][object] */ 


EXTERN_C const IID IID_IMcbCommPluginInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("48A185C2-FFDB-11d3-80E3-00C04F176153")
    IMcbCommPluginInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FormatErrorMessage( 
            /* [in] */ DWORD errCode,
            /* [string][out] */ LPOLESTR *pErrText) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMcbCommPluginInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMcbCommPluginInfo * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMcbCommPluginInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMcbCommPluginInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *FormatErrorMessage )( 
            IMcbCommPluginInfo * This,
            /* [in] */ DWORD errCode,
            /* [string][out] */ LPOLESTR *pErrText);
        
        END_INTERFACE
    } IMcbCommPluginInfoVtbl;

    interface IMcbCommPluginInfo
    {
        CONST_VTBL struct IMcbCommPluginInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMcbCommPluginInfo_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IMcbCommPluginInfo_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IMcbCommPluginInfo_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IMcbCommPluginInfo_FormatErrorMessage(This,errCode,pErrText)	\
    ( (This)->lpVtbl -> FormatErrorMessage(This,errCode,pErrText) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMcbCommPluginInfo_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


