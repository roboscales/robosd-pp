#ifndef __FREEMASTER_PLATFORM_DEFAULT_H
#define __FREEMASTER_PLATFORM_DEFAULT_H
/******************************************************************************
* platform-specific default configuration
******************************************************************************/

/* using 16bit addressing commands */
#ifndef FMSTR_USE_NOEX_CMDS
#define FMSTR_USE_NOEX_CMDS 1
#endif

/* using 32bit addressing commands */
#ifndef FMSTR_USE_EX_CMDS
#define FMSTR_USE_EX_CMDS   1
#endif

#ifndef FMSTR_ADDR2PTR
#define FMSTR_ADDR2PTR(addr)   ((FMSTR_U8 *)(FMSTR_ADDRESS_OFFSET + ((FMSTR_ADDRESS_OFFSET_TYPE)(addr))))
#endif

#ifndef FMSTR_PTR2ADDR
#define FMSTR_PTR2ADDR(ptr) ((FMSTR_ADDR)(((FMSTR_ADDRESS_OFFSET_TYPE)((FMSTR_U8 *)(ptr))) - FMSTR_ADDRESS_OFFSET))
#endif


/*****************************************************************************
* Board configuration information
******************************************************************************/

#define FMSTR_PROT_VER           3U      /* protocol version 3 */
#define FMSTR_CFG_FLAGS          0U      /* board info flags */
#define FMSTR_CFG_BUS_WIDTH      1U      /* data bus width */
#define FMSTR_GLOB_VERSION_MAJOR 2U      /* driver version */
#define FMSTR_GLOB_VERSION_MINOR 0U
#define FMSTR_IDT_STRING "ROBO SPACE DREAM FreeMASTER Driver"
#define FMSTR_TSA_FLAGS FMSTR_TSA_INFO_HV2BA    /* HawkV2 TSA workaround */

/******************************************************************************
* platform-specific types
******************************************************************************/

typedef unsigned char  FMSTR_U8;     /* smallest memory entity (mostly 8bit) */
typedef unsigned short FMSTR_U16;    /* 16bit value */
typedef unsigned long  FMSTR_U32;    /* 32bit value */

typedef signed char    FMSTR_S8;     /* signed 8bit value */
typedef signed short   FMSTR_S16;    /* signed 16bit value */
typedef signed long    FMSTR_S32;    /* signed 32bit value */

typedef unsigned short FMSTR_FLAGS;  /* type to be union-ed with flags (at least 8 bits) */
typedef unsigned short FMSTR_SIZE8;  /* size value (at least 8 bits) */
typedef signed short   FMSTR_INDEX;  /* general for-loop index (must be signed) */

typedef unsigned char  FMSTR_BCHR;   /* type of a single character in comm.buffer */
typedef unsigned char* FMSTR_BPTR;   /* pointer within a communication buffer */

typedef unsigned short FMSTR_SCISR;  /* data type to store SCI status register */

/******************************************************************************
* communication buffer access functions (non-inline)
******************************************************************************/

void FMSTR_CopyMemory(FMSTR_ADDR nDestAddr, FMSTR_ADDR nSrcAddr, FMSTR_SIZE8 nSize);
FMSTR_BPTR FMSTR_CopyToBuffer(FMSTR_BPTR pDestBuff, FMSTR_ADDR nSrcAddr, FMSTR_SIZE8 nSize);
FMSTR_BPTR FMSTR_CopyFromBuffer(FMSTR_ADDR nDestAddr, FMSTR_BPTR pSrcBuff, FMSTR_SIZE8 nSize);
void FMSTR_CopyFromBufferWithMask(FMSTR_ADDR nDestAddr, FMSTR_BPTR pSrcBuff, FMSTR_SIZE8 nSize);
FMSTR_BPTR FMSTR_AddressFromBuffer(FMSTR_ADDR* pAddr, FMSTR_BPTR pSrc);
FMSTR_BPTR FMSTR_AddressToBuffer(FMSTR_BPTR pDest, FMSTR_ADDR nAddr);

/* FMSTR_SetExAddr is needed only if both EX and non-EX commands are used */
#if FMSTR_USE_EX_CMDS && FMSTR_USE_NOEX_CMDS
void FMSTR_SetExAddr(FMSTR_BOOL bNextAddrIsEx);
#else
/* otherwise, we know what addresses are used, (ignore FMSTR_SetExAddr) */
#define FMSTR_SetExAddr(bNextAddrIsEx) 
#endif



FMSTR_BPTR   FMSTR_ValueFromBuffer16(FMSTR_U16* pDest, FMSTR_BPTR pSrc);

FMSTR_BPTR  FMSTR_ValueFromBuffer32(FMSTR_U32* pDest, FMSTR_BPTR pSrc);

FMSTR_BPTR  FMSTR_ValueToBuffer16(FMSTR_BPTR pDest, FMSTR_U16 src);

FMSTR_BPTR  FMSTR_ValueToBuffer32(FMSTR_BPTR pDest, FMSTR_U32 src);


/******************************************************************************
* communication buffer access inlines
******************************************************************************/

PLATFORM_INLINE FMSTR_BPTR  FMSTR_SkipInBuffer(FMSTR_BPTR pDest, FMSTR_SIZE8 nSize) {
	return pDest + nSize;
}

PLATFORM_INLINE FMSTR_BPTR  FMSTR_ValueFromBuffer8(FMSTR_U8* pDest, FMSTR_BPTR pSrc) {
	*pDest = *pSrc++;
	return pSrc;
}

PLATFORM_INLINE FMSTR_BPTR  FMSTR_ValueToBuffer8(FMSTR_BPTR pDest, FMSTR_U8 src) {
	*pDest++ = src;
	return pDest;
}

PLATFORM_INLINE FMSTR_BPTR  FMSTR_ConstToBuffer8(FMSTR_BPTR pDest, FMSTR_U8 src) {
	*pDest++ = src;
	return pDest;
}

PLATFORM_INLINE FMSTR_BPTR  FMSTR_ConstToBuffer16(FMSTR_BPTR pDest, FMSTR_U16 src) {
	return FMSTR_ConstToBuffer8(FMSTR_ConstToBuffer8(pDest, (FMSTR_U8)src), (FMSTR_U8)(src >> 8));
}

/****************************************************************************************
* memory access helper macros (used in recorder trigger compare routines)
*****************************************************************************************/

#define FMSTR_GetS8(addr)  ( *(FMSTR_S8*)FMSTR_ADDR2PTR(addr) )

#define FMSTR_GetU8(addr)  ( *(FMSTR_U8*)FMSTR_ADDR2PTR(addr) )

#define FMSTR_GetS16(addr) ( *(FMSTR_S16*)FMSTR_ADDR2PTR(addr) )

#define FMSTR_GetU16(addr) ( *(FMSTR_U16*)FMSTR_ADDR2PTR(addr) )

#define FMSTR_GetS32(addr) ( *(FMSTR_S32*)FMSTR_ADDR2PTR(addr) )

#define FMSTR_GetU32(addr) ( *(FMSTR_U32*)FMSTR_ADDR2PTR(addr) )

/****************************************************************************************
* Other helper macros
*****************************************************************************************/

/* This macro assigns C pointer to FMSTR_ADDR-typed variable */
/* should be done in assembly not to trim far-space pointers in SDM */

#define FMSTR_ARR2ADDR(tmpAddr, ptr) tmpAddr=FMSTR_PTR2ADDR(ptr)


/****************************************************************************************
* Platform-specific configuration check
*****************************************************************************************/

/* sanity check, at least one of the modes should be enabled */
#if (!FMSTR_USE_EX_CMDS) && (!FMSTR_USE_NOEX_CMDS)
#error At least one of FMSTR_USE_EX_CMDS or FMSTR_USE_NOEX_CMDS should be set non-zero
#endif

#define FMSTR_SCISR_TDRE 1
#define FMSTR_SCISR_RDRF 2
#endif /* __FREEMASTER_56F8xxx_H */
