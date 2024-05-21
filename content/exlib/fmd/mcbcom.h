/*******************************************************************************
*
* Copyright 2004-2015 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale FreeMASTER License
* distributed with this Material.
* See the LICENSE file distributed for more details.
* 
********************************************************************************
*
*   This is the main header file of the FreeMASTER communication library.
*
*******************************************************************************/

#ifndef _MCBCOM_H_
#define _MCBCOM_H_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MCBCOM_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MCBCOM_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#if defined(MCBCOM_STATIC)
#define MCBCOM_API 
#elif defined(MCBCOM_EXPORTS)
#define MCBCOM_API __declspec(dllexport) 
#else
#define MCBCOM_API __declspec(dllimport) 
#endif

/////////////////////////////////////////////////////
// handle to MCB communication port

typedef LPVOID HMCBCOM;

/////////////////////////////////////////////////////
// comm protocol Start Of Block character

#define MCB_SOBCHAR '+'
#define MCB_SOBSTR  "+"

/////////////////////////////////////////////////////
// comm protocol send commands

// note: assure not to develop command codes equal to one of their
//       possible response codes (single-wire-detection would not work)

// std packets CMDs with LEN byte following the CMD byte

#define MCB_CMD_READMEM			0x01
#define MCB_CMD_WRITEMEM		0x02
#define MCB_CMD_WRITEMEMMASK	0x03
#define MCB_CMD_READMEMEX		0x04	// v3+
#define MCB_CMD_WRITEMEMEX		0x05	// v3+
#define MCB_CMD_WRITEMEMMASKEX	0x06	// v3+
#define MCB_CMD_SETUPSCOPE		0x08
#define MCB_CMD_SETUPREC		0x09
#define MCB_CMD_SETUPSCOPEEX	0x0a	// v3+
#define MCB_CMD_SETUPRECEX		0x0b	// v3+
#define MCB_CMD_SENDAPPCMD		0x10
#define MCB_CMD_GETTSAINFO      0x11	// v3.1+ (zero data len)
#define MCB_CMD_GETTSAINFOEX    0x12	// v3.1+ (zero data len)
#define MCB_CMD_SFIOFRAME_1     0x13    // v3.2+ deliver & execute SFIO frame (even)
#define MCB_CMD_SFIOFRAME_0     0x14    // v3.2+ deliver & execute SFIO frame (odd)
#define MCB_CMD_PIPE            0x15	// v3.3+ lossless pipe I/O
#define MCB_CMD_INVALID			0x2b	// '+' is forbidden command number
#define MCB_CMD_RESERVED_FIRST	0xb0	// 
#define MCB_CMD_RESERVED_LAST	0xbf	// 0xb0 ... 0xbf are reserved for internal use

// fast cmd packets with set 7:6 bits implying the data part lenght at bits 5:4 

// no data part
#define MCB_CMD_GETINFO		    0xC0
#define MCB_CMD_STARTREC	    0xC1
#define MCB_CMD_STOPREC			0xC2
#define MCB_CMD_GETRECSTS		0xC3
#define MCB_CMD_GETRECBUFF		0xC4		
#define MCB_CMD_READSCOPE		0xC5
#define MCB_CMD_GETAPPCMDSTS	0xC6
#define MCB_CMD_CANCELAPPCMD	0xC7	// not yet used
#define MCB_CMD_GETINFOBRIEF	0xC8	// v2+
#define MCB_CMD_GETRECBUFFEX	0xC9	// v3+
#define MCB_CMD_SFIOGETRESP_0   0xca    // retry to get last SFIO response (even)
#define MCB_CMD_SFIOGETRESP_1   0xcb    // retry to get last SFIO response (odd)

// 2 bytes data part
#define MCB_CMD_READVAR8	0xD0
#define MCB_CMD_READVAR16	0xD1
#define MCB_CMD_READVAR32	0xD2
#define MCB_CMD_GETAPPCMDDATA	0xD3	// v4+ get the application command data
#define MCB_CMD_GETSTRLEN 		0xD4	// v4+ get string length (required by TSA)

// 4 bytes data part
#define MCB_CMD_READVAR8EX	0xE0
#define MCB_CMD_READVAR16EX	0xE1
#define MCB_CMD_READVAR32EX	0xE2
#define MCB_CMD_WRITEVAR8	0xE3	// v2+
#define MCB_CMD_WRITEVAR16	0xE4
#define MCB_CMD_WRITEVAR8MASK 0xE5
#define MCB_CMD_GETSTRLENEX   0xE6	// v4+

// 6 bytes data part
#define MCB_CMD_WRITEVAR32	   0xF0	// v2+
#define MCB_CMD_WRITEVAR16MASK 0xF1

/////////////////////////////////////////////////////
// comm protocol response status codes

/* flags in response codes */
#define MCB_STCF_ERROR            0x80    /* FLAG: error answer (no response data) */  
#define MCB_STCF_VARLEN           0x40    /* FLAG: variable-length answer (length byte) */  
#define MCB_STCF_EVENT            0x20    /* FLAG: reserved */  

// status codes signaling the successful response 
// (in terms of communication / command acception)
#define MCB_STC_OK			0x00	// operation successful
#define MCB_STC_RECRUN		0x01	// recorder running
#define MCB_STC_RECDONE		0x02	// recorder finished (done)

// status codes signaling some low-level error
#define MCB_STC_INVCMD		0x81	// invalid command (not implemented)
#define MCB_STC_CMDCSERR	0x82	// command checksum error
#define MCB_STC_CMDBUFFOVF	0x83	// command too long
#define MCB_STC_RSPBUFFOVF	0x84	// response would be too long
#define MCB_STC_INVBUFF		0x85	// invalid buffer length
#define MCB_STC_INVSIZE		0x86	// invalid size operand
#define MCB_STC_SERVBUSY	0x87	// service is busy
#define MCB_STC_NOTINIT		0x88	// service not initialised
#define MCB_STC_EACCESS     0x89	// access denied
#define MCB_STC_SFIOERR     0x8A	// SFIO error
#define MCB_STC_UNKNOWN		0xFF	// reserved

//////////////////////////////////////////////////////
// feature identifiers for calling McbSendRecvPacket.
// each defined feature can be locked when using shared
// RS232 access via remote server (mcbsvr)

#define MCB_FEATURE_SCOPE	0x01
#define MCB_FEATURE_REC		0x02
#define MCB_FEATURE_APPCMD	0x03
#define MCB_FEATURE_FPROG	0x04
#define MCB_FEATURE_SFIO	0x05
#define MCB_FEATURE_PIPES	0x06

/////////////////////////////////////////////////////
// Flash Programmer commands / status bytes MSB is 
// the "COMMAND" bit which forces embedded side operation

#define MCB_FPROG_CMD_PENDING	0x80	// command/status exchange bit
#define MCB_FPROG_CMD_32ADDR	0x08	// using 32bit address flag

#define MCB_FPROG_CMD_READ		(0x00 | MCB_FPROG_CMD_PENDING)
#define MCB_FPROG_CMD_BCHECK	(0x01 | MCB_FPROG_CMD_PENDING)
#define MCB_FPROG_CMD_ERASE		(0x02 | MCB_FPROG_CMD_PENDING)
#define MCB_FPROG_CMD_WRITE		(0x03 | MCB_FPROG_CMD_PENDING)
#define MCB_FPROG_CMD_BLKINFO	(0x04 | MCB_FPROG_CMD_PENDING)

// flags to be combined with FPROG_CMD_ to specify Flash type
#define MCB_FPROG_CMD_FTYPE0	0x00
#define MCB_FPROG_CMD_FTYPE1	0x10
#define MCB_FPROG_CMD_FTYPE2	0x20
#define MCB_FPROG_CMD_FTYPE3	0x30
#define MCB_FPROG_CMD_FTYPE4	0x40
#define MCB_FPROG_CMD_FTYPE5	0x50
#define MCB_FPROG_CMD_FTYPE6	0x60
#define MCB_FPROG_CMD_FTYPE7	0x70
#define MCB_FPROG_CMD_FTYPE(n)	(((n)<<4)&0x70)

// return status codes (MSB cleared to signal "done" state)
#define MCB_FPROG_STS_MASK		0x0f	// bits[6:4] in status byte are reserved
#define MCB_FPROG_STS_SUCC		0x00	// operation finished successfuly
#define MCB_FPROG_STS_FAIL		0x01	// fail: generic fail
#define MCB_FPROG_STS_NOACC		0x02	// fail: access denied
#define MCB_FPROG_STS_INVPRM	0x03	// fail: invalid parameters passed
#define MCB_FPROG_STS_MISSALIGN	0x04	// fail: misaligned access

/////////////////////////////////////////////////////
// DLL exported functions return codes (it is the 32 bit
// unsigned integer, highest bit indicates the error
// lower bits are the specific result code

// all the constants are defined in the separate message heared file

#include "mcberr.h"

/*
// success codes equivalent to protocol status codes 
#define MCB_S_OK			0				// unspecified success
#define MCB_S_RECRUN		0x01			// recorder running
#define MCB_S_RECDONE		0x02			// recorder done

// error codes equivalent to protocol status codes 
#define MCB_E_INVCMD		0x80000081		// invalid command
#define MCB_E_CMDCSERR		0x80000082		// command checksum err
#define MCB_E_CMDTOOLONG	0x80000083		// command too long
#define MCB_E_RSPBUFFOVF	0x80000084		// response buffer overflow
#define MCB_E_INVBUFF		0x80000085		// invalid buffer length (eg. rec)
#define MCB_E_INVSIZE		0x80000086		// invalid size (eg. rec var on 16bit)
#define MCB_E_SERVBUSY		0x80000087		// service is busy
#define MCB_E_NOTINIT		0x80000088		// service not initialised
#define MCB_E_EACCESS       0x80000089		// access to target resource is denied
#define MCB_E_SFIOERR		0x8000008A		// error returned by SFIO system
#define MCB_E_SFIOUNMATCH	0x8000008B		// mismatch in SFIO request/response sequence
#define MCB_E_UNKNOWN		0x800000FF		// unknown error

// other error codes (system, etc)
#define MCB_E_CANTWRITE		0x80000100		// port write error
#define MCB_E_RESPTMOUT		0x80000101		// no response (timeout)
#define MCB_E_RESPCSERR		0x80000102		// response checksum err
#define MCB_E_CANTDETECT	0x80000103		// cannot detect the hardware
#define MCB_E_BUFFSMALL		0x80000104		// buffer size too small
#define MCB_E_INVPOINTER	0x80000105		// buffer pointer invalid
#define MCB_E_LIMITEXCDED	0x80000106		// limit of entities exceeded
#define MCB_E_A32NOTSUPP	0x80000107		// 32 bit adresses not supported
#define MCB_E_D32NOTSUPP	0x80000108		// 32 bit data values not supp.
#define MCB_E_ERRSIZE		0x80000109		// error in size specification
#define MCB_E_ERRSIZEBUS	0x8000010A		// size incompatible with bus width
#define MCB_E_ERRDATA		0x8000010B		// error in entry data
#define MCB_E_RESPWRONG		0x8000010C		// error in response
#define MCB_E_FPROGERROR	0x800002xx		// flash programmer error (xx is status with 0x7f mask)
#define MCB_E_FPROGINVBSZ	0x80000280		// FP config error, buffer size error
#define MCB_E_FPROGINVCSZ	0x80000281		// FP config error, CSW size error
#define MCB_E_FPROGTMOUT	0x80000282		// FP operation timoeut
#define MCB_E_FPROGNOCFG	0x80000283		// FP is not yet configured
#define MCB_E_FPROGADDR32BIT 0x80000284		// FP needs to use 32bit addressing
#define MCB_E_FPROGBUSY 	0x80000285		// FP is still processing previous command
#define MCB_E_FPROGABORTED 	0x80000286		// FP operation aborted by callback function
#define MCB_E_SFIOUNEX  	0x80000287		// unexpected SFIO behavior
#define MCB_E_PORTCLOSED	0x8000FFFB		// port is not opened
#define MCB_E_OBSOLETED		0x8000FFFC		// library function obsoleted
#define MCB_E_TOOMANYTHRDS	0x8000FFFD		// too many threads accessing remote server
#define MCB_E_NOTIMPL		0x8000FFFE		// not implemented 
#define MCB_E_UNEXPECTED	0x8000FFFF		// unexpected error

*/

#define MCB_FAILED(code)	FAILED(code)
#define MCB_SUCCEEDED(code)	SUCCEEDED(code)
#define MCB_GETSTS(code)	((code) & 0xffff)

/////////////////////////////////////////////////////
// data types

// single byte alignment
#pragma pack(push, mcb, 1)

// response to MCB_CMD_GETINFO command (single byte items - byte order 
// might be unknown when reading/parsing this structure)

#define MCB_DESCR_SIZE 25
#define MCB_CFGFLAG_BIGENDIAN	0x01	// board uses bigEndian byte order
#define MCB_CFGFLAG_NOFASTREAD	0x02	// do not try the fast read commands v2+
#define MCB_CFGFLAG_NOFASTWRITE	0x04	// do not try the fast write commands v2+
#define MCB_CFGFLAG_EXACCESSONLY 0x08	// do not try 16-bit address access v3+

typedef struct
{
	BYTE protVer;				// protocol version
	BYTE cfgFlags;				// MCB_CFGFLAG_ bits
	BYTE dataBusWdt;			// data bus width (bytes)
	BYTE globVerMajor;			// board firmware version major number
	BYTE globVerMinor;			// board firmware version minor number
	BYTE cmdBuffSize;			// receive/transmit buffer size (without SOB, CMD/STS and CS)
	WORD recBuffSize;			// recorder buffer memory
	WORD recTimeBase;			// recorder time base (2 MSB exponent 1 = m, 2 = u, 3 = n)
	BYTE descr[MCB_DESCR_SIZE]; // ANSI string, board description

} MCB_RESP_GETINFO, FAR* LPMCB_RESP_GETINFO;

// to minimize transmit buffer memory needs on target board, the protocol
// versions 2+ allows target to implement a brief get-info command, with the 
// subset of standard get-info information

// it has the same structure as the head of standard get-info command response

typedef struct
{
	BYTE protVer;				// protocol version
	BYTE cfgFlags;				// MCB_CFGFLAG_ bits
	BYTE dataBusWdt;			// data bus width (bytes)
	BYTE globVerMajor;			// board firmware version major number
	BYTE globVerMinor;			// board firmware version minor number
	BYTE cmdBuffSize;			// receive/transmit buffer size (without SOB, CMD/STS and CS)

} MCB_RESP_GETINFOBRIEF, FAR* LPMCB_RESP_GETINFOBRIEF;

// response to MCB_CMD_GETAPPCMDSTS command

#define MCB_APPCMDRESULT_NOCMD		0xff
#define MCB_APPCMDRESULT_RUNNING	0xfe
#define MCB_APPCMDRESULT_LASTVALID	0xf7	// F8-FF is reserved code space

typedef struct 
{
	BYTE result;				// command return value

} MCB_RESP_GETAPPCMDSTS, FAR* LPMCB_RESP_GETAPPCMDSTS;

// data passed to MCB_CMD_SENDAPPCMD command

typedef struct 
{
	BYTE code;
	BYTE args[1];

} MCB_DATA_SENDAPPCMD, FAR* LPMCB_DATA_SENDAPPCMD;

// data passed to MCB_CMD_GETAPPCMDDATA command

typedef struct 
{
	BYTE length;
	BYTE offset;

} MCB_DATA_GETAPPCMDDATA, FAR* LPMCB_DATA_GETAPPCMDDATA;

// data passed to MCB_CMD_READMEM command

typedef struct
{
	BYTE size;			// required size
	WORD addr;			// memory block address

} MCB_DATA_READMEM, FAR* LPMCB_DATA_READMEM;

// data passed to MCB_CMD_READMEMEX command

typedef struct
{
	BYTE size;			// required size
	DWORD addr;			// memory block address

} MCB_DATA_READMEMEX, FAR* LPMCB_DATA_READMEMEX;

// data passed to MCB_CMD_WRITEMEM or MCB_CMD_WRITEMEMMASK command

typedef struct
{
	BYTE size;			// write buffer size
	WORD addr;			// dest memory address
	BYTE buffer[1];		// data block follows

} MCB_DATA_WRITEMEM, FAR* LPMCB_DATA_WRITEMEM;

// data passed to MCB_CMD_WRITEMEMEX or MCB_CMD_WRITEMEMMASKEX command

typedef struct
{
	BYTE size;			// write buffer size
	DWORD addr;			// dest memory address
	BYTE buffer[1];		// data block follows

} MCB_DATA_WRITEMEMEX, FAR* LPMCB_DATA_WRITEMEMEX;

// data passed to MCB_CMD_WRITEVAR8 and MCB_CMD_WRITEVAR16 (protocol version 2+)
// note that it is the same for 8 and 16 bits ops (fast commands cannot carry 
// 3-bytes data, so the 8-bit write is little bit inefficient)

typedef struct
{
	WORD addr;			// memory location address
	union {
		BYTE val_b;		// 8-bit value for MCB_CMD_WRITEVAR8
		WORD val_w;		// 16-bit value for MCB_CMD_WRITEVAR16
	};

} MCB_DATA_WRITEVAR, FAR* LPMCB_DATA_WRITEVAR;

typedef struct
{
	WORD addr;			// memory location address
	DWORD val;			// 32-bit value

} MCB_DATA_WRITEVAR32, FAR* LPMCB_DATA_WRITEVAR32;

// data structures for fast masked writes

typedef struct
{
	WORD addr;			// memory location address
	BYTE val;			// variable value
	BYTE mask;			// mask value

} MCB_DATA_WRITEVAR8MASK, FAR* LPMCB_DATA_WRITEVAR8MASK;

typedef struct
{
	WORD addr;			// memory location address
	WORD val;			// variable value
	WORD mask;			// mask value

} MCB_DATA_WRITEVAR16MASK, FAR* LPMCB_DATA_WRITEVAR16MASK;

// common substructures entries for setting up scope and recorder

typedef struct { 
	BYTE size;		// variable size
	WORD addr;		// variable address
} MCB_DATA_SETUPSCOPE_VARDEF;

typedef struct { 
	BYTE size;		// variable size
	DWORD addr;		// variable address
} MCB_DATA_SETUPSCOPEEX_VARDEF;

// data passed to MCB_CMD_SETUPSCOPE command

typedef struct
{
	BYTE varCnt;		// number of valid var definitions
	MCB_DATA_SETUPSCOPE_VARDEF varDef[1];

} MCB_DATA_SETUPSCOPE, FAR* LPMCB_DATA_SETUPSCOPE;

// data passed to MCB_CMD_SETUPSCOPEEX command

typedef struct
{
	BYTE varCnt;		// number of valid var definitions
	MCB_DATA_SETUPSCOPEEX_VARDEF varDef[1];

} MCB_DATA_SETUPSCOPEEX, FAR* LPMCB_DATA_SETUPSCOPEEX;

// data passed to MCB_CMD_SETUPREC command

typedef struct
{
	BYTE trgMode;			// trigger mode (0 = disabled, 1 = _/ , 2 = \_)
	WORD totalSmps;			// buffer size in samples
	WORD postTrigger;		// number of samples after trigger to save
	WORD timeDiv;			// time base unit multiplier (0 = fastest)

	WORD trgVarAddr;		// trigger variable address
	BYTE trgVarSize;		// trigger variable/threshold size (1,2,4)
	BYTE trgVarSigned;		// trigger compare mode (0 = unsigned, 1 = signed)
	union {
		BYTE b;				// trgVarSize == 1
		WORD w;				// trgVarSize == 2
		DWORD dw;			// trgVarSize == 4
	} trgThreshold;			// trigger comparing threshold

	MCB_DATA_SETUPSCOPE vars;	// recorded variables

} MCB_DATA_SETUPREC, FAR* LPMCB_DATA_SETUPREC;

// data passed to MCB_CMD_SETUPRECEX command

typedef struct
{
	BYTE trgMode;			// trigger mode (0 = disabled, 1 = _/ , 2 = \_)
	WORD totalSmps;			// buffer size in samples
	WORD postTrigger;		// number of samples after trigger to save
	WORD timeDiv;			// time base unit multiplier (0 = fastest)

	DWORD trgVarAddr;		// trigger variable address
	BYTE trgVarSize;		// trigger variable/threshold size (1,2,4)
	BYTE trgVarSigned;		// trigger compare mode (0 = unsigned, 1 = signed)
	union {
		BYTE b;				// trgVarSize == 1
		WORD w;				// trgVarSize == 2
		DWORD dw;			// trgVarSize == 4
	} trgThreshold;			// trigger comparing threshold

	MCB_DATA_SETUPSCOPEEX vars;	 // recorded variables

} MCB_DATA_SETUPRECEX, FAR* LPMCB_DATA_SETUPRECEX;

// response to MCB_CMD_GETRECBUFF command

typedef struct 
{
	WORD buffAddr;			// cyclic buffer address
	WORD startIx;			// first sample index

} MCB_RESP_GETRECBUFF, FAR* LPMCB_RESP_GETRECBUFF;

// response to MCB_CMD_GETRECBUFFEX command

typedef struct 
{
	DWORD buffAddr;			// cyclic buffer address
	WORD startIx;			// first sample index

} MCB_RESP_GETRECBUFFEX, FAR* LPMCB_RESP_GETRECBUFFEX;

// response to MCB_CMD_GETTSAINFO command

#define MCB_TSAINFOFLAG_VERSION_MASK  0x000f
#define MCB_TSAINFOFLAG_32BIT  0x0100	// address format (16/32 bit)
#define MCB_TSAINFOFLAG_HV2BA  0x0200	// HawkV2 byte-addressing fix

typedef struct 
{
	WORD tsaFlags;
	WORD tblSize;
	WORD tblAddr;

} MCB_RESP_GETTSAINFO, FAR* LPMCB_RESP_GETTSAINFO;

// response to MCB_CMD_GETTSAINFOEX command

typedef struct 
{
	WORD tsaFlags;
	WORD tblSize;
	DWORD tblAddr;

} MCB_RESP_GETTSAINFOEX, FAR* LPMCB_RESP_GETTSAINFOEX;

// Target-side Address translation structures

typedef struct
{
	WORD  name;
	WORD  type;
	WORD  addr;
	WORD  info;
} MCB_TSA_ENTRY16, FAR* LPMCB_TSA_ENTRY16;

typedef struct
{
	DWORD  name;
	DWORD  type;
	DWORD  addr;
	DWORD  info;
} MCB_TSA_ENTRY32, FAR* LPMCB_TSA_ENTRY32;

#define MCB_TSA_UINT8   ((CHAR)0xE0U)
#define MCB_TSA_UINT16  ((CHAR)0xE1U)
#define MCB_TSA_UINT32  ((CHAR)0xE2U)
#define MCB_TSA_UINT64  ((CHAR)0xE3U)
#define MCB_TSA_SINT8   ((CHAR)0xF0U)
#define MCB_TSA_SINT16  ((CHAR)0xF1U)
#define MCB_TSA_SINT32  ((CHAR)0xF2U)
#define MCB_TSA_SINT64  ((CHAR)0xF3U)
#define MCB_TSA_UFRAC16 ((CHAR)0xE5U)
#define MCB_TSA_UFRAC32 ((CHAR)0xE6U)
#define MCB_TSA_FRAC16  ((CHAR)0xF5U)
#define MCB_TSA_FRAC32  ((CHAR)0xF6U)
#define MCB_TSA_FLOAT   ((CHAR)0xFAU)
#define MCB_TSA_DOUBLE  ((CHAR)0xFBU)
#define MCB_TSA_INVALID ((CHAR)0xFFU)
#define MCB_TSA_SPECIAL ((CHAR)0xECU) // special information (not a symbol)

#define MCB_TSA_ISBASETYPE(type)  (((type) & 0xE0) == 0xE0U)
#define MCB_TSA_GETBTYPESIZE(type) (1 << ((type) & 0x3U)) // only for types without additional format info
#define MCB_TSA_ISSIGNED(type)    ((type) & 0x10U)
#define MCB_TSA_ISINTEGER(type)   (((type) & 0xcU) == 0x0U)
#define MCB_TSA_ISFRACT(type)     (((type) & 0xcU) == 0x4U)
#define MCB_TSA_ISREAL(type)      (((type) & 0xcU) == 0x8U)

// TSA flags carried in TSA_ENTRY.info 
#define FMSTR_TSA_INFO_ENTRYTYPE_MASK 0x0003U 
#define FMSTR_TSA_INFO_STRUCT         0x0000U
#define FMSTR_TSA_INFO_RO_VAR         0x0001U 
#define FMSTR_TSA_INFO_MEMBER         0x0002U
#define FMSTR_TSA_INFO_RW_VAR         0x0003U 

#define FMSTR_TSA_ISSTRUCT(info)   (((info) & FMSTR_TSA_INFO_ENTRYTYPE_MASK) == FMSTR_TSA_INFO_STRUCT)
#define FMSTR_TSA_ISMEMBER(info)   (((info) & FMSTR_TSA_INFO_ENTRYTYPE_MASK) == FMSTR_TSA_INFO_MEMBER)
#define FMSTR_TSA_ISVAR(info)      (((info) & FMSTR_TSA_INFO_RO_VAR) == FMSTR_TSA_INFO_RO_VAR)
#define FMSTR_TSA_GETOBJSIZE(info) ((info) >> 2)

// the special type information is coded as string ":TYPE" which follows the MCB_TSA_SPECIAL character
#define MCB_TSA_SPECIAL_PRJ  "PRJ"
#define MCB_TSA_SPECIAL_DIR  "DIR"
#define MCB_TSA_SPECIAL_FILE "MEMFILE"
#define MCB_TSA_SPECIAL_HREF "HREF"

#pragma pack(pop, mcb)

/////////////////////////////////////////////////////
// Mcb function calls data types

#pragma pack(push, mcb2, 8)

// scope/rec setup

typedef struct { 
	BYTE size;			// variable remote size
	DWORD addr;			// variable remote address 
	LPVOID destPtr;		// destination buffer
	DWORD  destSize;	// destination buffer size (robust checking)

} MCB_SCOPE, FAR* LPMCB_SCOPE;

// recorder setup

typedef struct { 
	BYTE trgMode;			// trigger mode (0 = disabled, 1 = _/ , 2 = \_ , 3 = both)
	WORD totalSmps;			// total number of samples required
	WORD preTrigger;		// number of samples of pretrigger
	WORD timeDiv;			// time base unit multiplier
	DWORD trgVarAddr;		// trigger variable address
	BYTE  trgVarSize;		// trigger variable size (1, 2 or 4)
	BYTE  trgVarSigned;		// signed flag (0 = unsigned, 1 = signed)

	union {
		BYTE b;				// trgVarSize == 1
		WORD w;				// trgVarSize == 2
		DWORD dw;			// trgVarSize == 4
	} trgThreshold;			// trigger comparing threshold

} MCB_RECORDER, FAR* LPMCB_RECORDER;

// flash programmer operation

typedef struct {
	DWORD cmd;				// FPROG_xx command to execute
	DWORD addr;				// destination Flash address
	DWORD size;				// size (in byte) of data passed in dataIn or expected in dataOut (or both)
	LPCVOID dataIn;			// data to be uploaded before command execution
	LPVOID dataOut;			// destination data buffer
} MCB_FPROGOPERATION, FAR* LPMCB_FPROGOPERATION;

// flash programmer progress callback function type

typedef BOOL (CALLBACK *MCB_FPROGCALLBACK)(LPMCB_FPROGOPERATION pOperation, DWORD dwAddrNow, DWORD dwToDoBytes, LPVOID pParam);

// flags for MCB_FPROGSETUP "mask" member

#define MCB_FPS_CSW	 0x0001U	// csw* members
#define MCB_FPS_BUFF 0x0002U	// buff* and bExAddr members
#define MCB_FPS_TMO  0x0004U	// timeout member
#define MCB_FPS_PCB  0x0008U	// progress callback members

// flash programmer setup 

typedef struct {
	DWORD mask;				// mask of what is valid in this structure (MCB_FPS_xxx bits)
	DWORD cswAddr;			// CSW address
	WORD  cswSize;			// CSW size (typically 1)
	DWORD buffAddr;			// I/O Buffer address
	WORD  buffSize;			// I/O Buffer size
	BOOL  bExAddr;			// use 32bit address in I/O buffer
	DWORD timeout;			// flash operation timeout [ms]
	MCB_FPROGCALLBACK pProgressCB; // progress callback
	LPVOID pProgressParam;	// progress callback cookie parameter
} MCB_FPROGSETUP, FAR* LPMCB_FPROGSETUP;

// TSA info

typedef struct 
{
	WORD tsaFlags;
	WORD tblSize;
	DWORD tblAddr;
} MCB_TSAINFO, FAR* LPMCB_TSAINFO;

// cached SFIO response

#define MCB_MAX_SFIODATA_LEN 254

typedef struct {
	WORD size;
	BYTE data[MCB_MAX_SFIODATA_LEN];
} MCB_SFIORESPONSE, FAR* LPMCB_SFIORESPONSE;

// PIPE write frame

typedef struct {
	BYTE portAndToggle;	// pipe port, 0x80 bit toggles each transmission
	BYTE rxAck; 		// tells the remote side how many bytes accepted in the last transaction
	BYTE data[1];
} MCB_PIPEWRITE, FAR* LPMCB_PIPEWRITE;

typedef struct {
	BYTE frameLength;	// variable frame length
	BYTE portAndToggle;	// pipe port, 0x80 bit toggles each transmission
	BYTE rxAck; 		// tells the PC side how many bytes accepted in the last transaction
	BYTE data[1];
} MCB_PIPERESPONSE, FAR* LPMCB_PIPERESPONSE;

#pragma pack(pop, mcb2)

/////////////////////////////////////////////////////
// DLL exported functions

#ifdef __cplusplus
extern "C" {
#endif

// port open/close
MCBCOM_API DWORD McbOpenCom(HMCBCOM* phCom, int port, int speed = 9600, LPWSTR remoteServer = NULL);
MCBCOM_API DWORD McbOpenComEx(HMCBCOM* phCom, LPCOLESTR connectString);
MCBCOM_API DWORD McbAttachThread(HMCBCOM hCom);
MCBCOM_API void McbDetachThread(HMCBCOM hCom);
MCBCOM_API void McbCloseCom(HMCBCOM hCom);

// hardware detection
MCBCOM_API DWORD McbDetect(HMCBCOM hCom, LPMCB_RESP_GETINFO pinfo);
MCBCOM_API DWORD McbLinSyncAndDetect(HMCBCOM hCom, LPMCB_RESP_GETINFO pinfo);
MCBCOM_API DWORD McbGetInfo(HMCBCOM hCom, LPMCB_RESP_GETINFO pinfo);
MCBCOM_API DWORD McbMtoH(HMCBCOM hCom, LPVOID buff, WORD size);
MCBCOM_API DWORD McbHtoM(HMCBCOM hCom, LPVOID buff, WORD size);

// send any data using PCMaster protocol (advanced users only)
MCBCOM_API DWORD McbSendPacket(HMCBCOM hCom, BYTE cmd, LPCVOID buff, WORD size);
MCBCOM_API DWORD McbRecvResponse(HMCBCOM hCom, LPVOID buff, WORD size);

// get variable (universal)
MCBCOM_API DWORD McbReadVar(HMCBCOM hCom, LPVOID dest, DWORD addr, WORD size);

// get variable (fast comm)
MCBCOM_API DWORD McbReadVar8(HMCBCOM hCom, LPVOID dest, DWORD addr);
MCBCOM_API DWORD McbReadVar16(HMCBCOM hCom, LPVOID dest, DWORD addr);
MCBCOM_API DWORD McbReadVar32(HMCBCOM hCom, LPVOID dest, DWORD addr);

// write variable (universal)
MCBCOM_API DWORD McbWriteVar(HMCBCOM hCom, LPCVOID src, DWORD addr, WORD size);
MCBCOM_API DWORD McbWriteVarBits(HMCBCOM hCom, LPCVOID src, LPCVOID mask, DWORD addr, WORD size);

// set & read the scope
MCBCOM_API DWORD McbSetupScope(HMCBCOM hCom, int varsCnt, LPMCB_SCOPE ps);
MCBCOM_API DWORD McbReadScope(HMCBCOM hCom);

// set, check & read the recorder
MCBCOM_API DWORD McbSetupRec(HMCBCOM hCom, int varsCnt, LPMCB_SCOPE ps, LPMCB_RECORDER pr);
MCBCOM_API DWORD McbStartRec(HMCBCOM hCom);
MCBCOM_API DWORD McbStopRec(HMCBCOM hCom);
MCBCOM_API DWORD McbGetRecStatus(HMCBCOM hCom);
MCBCOM_API DWORD McbReadRec(HMCBCOM hCom);

// data block operations
MCBCOM_API DWORD McbReadDataMem(HMCBCOM hCom, LPVOID dest, DWORD addr, WORD size);
MCBCOM_API DWORD McbWriteDataMem(HMCBCOM hCom, LPCVOID src, DWORD addr, WORD size);
MCBCOM_API DWORD McbWriteDataMemMask(HMCBCOM hCom, LPCVOID src, LPCVOID mask, DWORD addr, WORD size);

// application commands (McbGetAppCmdData added Sep 2005)
MCBCOM_API DWORD McbSendAppCmd(HMCBCOM hCom, BYTE code, DWORD argSize, LPCVOID argBuff);
MCBCOM_API DWORD McbGetAppCmdStatus(HMCBCOM hCom, LPBYTE pCmdStatus);
MCBCOM_API DWORD McbGetAppCmdData(HMCBCOM hCom, LPVOID dest, WORD size, WORD offset);
MCBCOM_API DWORD McbCancelAppCmd(HMCBCOM hCom);

// added in V2.1 (29-Sep 2000) (mcbsvr sharing port features)
MCBCOM_API DWORD McbSendRecvPacket(HMCBCOM hCom, BYTE cmd, LPCVOID cmdBuff, WORD cmdSize, LPVOID respBuff, WORD respSize, BYTE featureId);
MCBCOM_API DWORD McbLockFeature(HMCBCOM hCom, BYTE featureId);
MCBCOM_API DWORD McbUnlockFeature(HMCBCOM hCom, BYTE featureId);
MCBCOM_API int McbGetErrorMessage(DWORD errCode, LPSTR buff, int maxLen);

// added (24-Mar 2003)
MCBCOM_API int McbGetErrorMessage2(HMCBCOM hCom, DWORD errCode, LPTSTR buff, int maxLen);

// added (21-Apr 2004) Flash Programmer interface
MCBCOM_API DWORD McbFlashProgSetup(HMCBCOM hCom, LPMCB_FPROGSETUP pfps);
MCBCOM_API DWORD McbFlashProgGetConfig(HMCBCOM hCom, LPMCB_FPROGSETUP pfps);
MCBCOM_API DWORD McbFlashProgOperation(HMCBCOM hCom, LPMCB_FPROGOPERATION pfpo);

// added (26-May 2005) Target-Side Address Translation support
MCBCOM_API DWORD McbGetTargetSideSymbolInfo(HMCBCOM hCom, WORD tblIndex, LPMCB_TSAINFO ptsai);
MCBCOM_API DWORD McbGetStringLength(HMCBCOM hCom, DWORD addr, DWORD* psize);

// added (Sep 2005) Plugin Manipulation
enum MCB_PLUGIN_STRING
{
	PS_CLSID, // CLSID as string
	PS_USER,  // Friendly name

	PS_MRU_CS_FIRST = 20000,
	PS_MRU_CS_LAST  = 20999,
	PS_STD_CS_FIRST = 21000,
	PS_STD_CS_LAST  = 21999,
};

typedef BOOL (CALLBACK * MCB_ENUMPLUGINFUNC)(CLSID*, LPVOID);
MCBCOM_API HRESULT McbEnumPlugins(MCB_ENUMPLUGINFUNC pEnumFunc, LPVOID pParam);
MCBCOM_API HRESULT McbConfigurePlugin(HWND hWndParent, CLSID* pCLSID, LPOLESTR pInOutParams, LONG nMaxLen);
MCBCOM_API HRESULT McbGetPluginString(CLSID* pCLSID, LONG nStrId, LPOLESTR pStr, LPLONG pMaxLen);

MCBCOM_API HRESULT McbBuildPluginConnectString(CLSID* pCLSID, LPCOLESTR pInParams, LPOLESTR pOutCS, LPLONG pMaxLen);
MCBCOM_API HRESULT McbBuildRS232ConnectString(LONG nCOM, const DCB* pInDCB, const COMMTIMEOUTS* pInTMO, LPOLESTR pOutCS, LPLONG pMaxLen);
MCBCOM_API HRESULT McbGetRS232Defaults(DCB* pOutDCB, COMMTIMEOUTS* pOutTMO);

MCBCOM_API HRESULT McbIsPluginConnectString(LPCOLESTR pInCS, CLSID* pOutCLSID, LPCOLESTR* ppOutParams);
MCBCOM_API HRESULT McbIsPluginConnectString2(LPCOLESTR pInCS, CLSID* pOutCLSID, LPOLESTR pOutParams, LPLONG pMaxLen);
MCBCOM_API HRESULT McbIsRS232ConnectString(LPCOLESTR pInCS, LONG* pOutCOM, DCB* pOutDCB, COMMTIMEOUTS* pOutTMO);
MCBCOM_API HRESULT McbIsNamedConnection(LPCOLESTR pInCS, BOOL* pIsGlobal, LPOLESTR pOutCS, LPLONG pMaxLen);

MCBCOM_API HRESULT McbEnumNamedConnection(DWORD dwIndex, BOOL bGlobal, LPOLESTR pName, LPLONG pMaxLen);
MCBCOM_API HRESULT McbSaveNamedConnection(LPCOLESTR pName, BOOL bGlobal, LPCOLESTR pInCS);
MCBCOM_API HRESULT McbLoadNamedConnection(LPCOLESTR pName, BOOL bGlobal, LPOLESTR pOutCS, LPLONG pMaxLen);
MCBCOM_API HRESULT McbDeleteNamedConnection(LPCOLESTR pName, BOOL bGlobal);

// added (May 2006) SFIO Matlab interfacing
MCBCOM_API DWORD McbSendSfioFrame(HMCBCOM hCom, LPCVOID pFrame, WORD nSize);
MCBCOM_API DWORD McbReadSfioResponse(HMCBCOM hCom, LPMCB_SFIORESPONSE pDest, WORD nExpectedSize);

// added (Jul 2006) Pipe lossless communication
MCBCOM_API DWORD McbPipeOpen(HMCBCOM hCom, WORD nPort, DWORD dwTxBufferSize, DWORD dwRxBufferSize);
MCBCOM_API DWORD McbPipeGetBufferSizes(HMCBCOM hCom, WORD nPort, DWORD* pdwTxBufferSize, DWORD* pdwRxBufferSize);
MCBCOM_API DWORD McbPipeClose(HMCBCOM hCom, WORD nPort);
MCBCOM_API DWORD McbPipeWrite(HMCBCOM hCom, WORD nPort, LPCVOID pData, DWORD dwBytesToWrite, DWORD dwGranularity, DWORD* pdwBytesWritten);
MCBCOM_API DWORD McbPipeFlush(HMCBCOM hCom, WORD nPort, LONG nFlushTimeout, DWORD* pdwTxBytesPending);
MCBCOM_API DWORD McbPipeRead(HMCBCOM hCom, WORD nPort, LPVOID pBuff, DWORD dwBytesToRead, DWORD dwGranularity, LONG nFlushTimeout, DWORD* pdwBytesRead);

// added (Jul 2014) Communication Logging
MCBCOM_API HRESULT McbEnableLogging(DWORD dwSize);
MCBCOM_API HRESULT McbReadLogLine(LPTSTR pszDest, int nMaxSize, int* pnCharsRead);
MCBCOM_API HRESULT McbSetLogVerbosity(int nVerbosity);
MCBCOM_API HRESULT McbSetLogServices(DWORD dwMask, DWORD dwServices);
MCBCOM_API HRESULT McbSetLogStatFilter(DWORD dwMask, DWORD dwEnable);
MCBCOM_API HRESULT McbSetLogTextFilter(LPCTSTR pszWords);
MCBCOM_API HRESULT McbLogBeginRecord(DWORD dwService, int nVerbosity, LPCTSTR pszText);
MCBCOM_API HRESULT McbLogEndRecordText(LPCTSTR pszText, DWORD dwLogTransStatus); // use with MCB_LOGTRANS_STAT_xxx
MCBCOM_API HRESULT McbLogEndRecordResult(HMCBCOM hCom, HRESULT resultCode);      // hCom may be NULL, resultCode is one return codes
MCBCOM_API HRESULT McbLogText(int nVerbosity, LPCSTR pszText, ...);
MCBCOM_API HRESULT McbLogTextW(int nVerbosity, LPCWSTR pszText, ...);

#define MCB_LOGSVC_PORT   0x0001UL
#define MCB_LOGSVC_READ   0x0002UL
#define MCB_LOGSVC_WRITE  0x0004UL
#define MCB_LOGSVC_SCOPE  0x0008UL
#define MCB_LOGSVC_REC    0x0010UL
#define MCB_LOGSVC_APPCMD 0x0020UL
#define MCB_LOGSVC_SFIO   0x0040UL
#define MCB_LOGSVC_TSA    0x0080UL
#define MCB_LOGSVC_PIPE   0x0100UL
#define MCB_LOGSVC_POLL   0x0200UL // polling commands (scope, rec, appcmd, pipe, sfio, ...)
#define MCB_LOGSVC_ALL    0xffffUL // useful services are kept in lower 16bits
#define MCB_LOGSVC_ALWAYS 0x80000000UL // general service which is always logged

#define MCB_LOGVERB0_OFF  0
#define MCB_LOGVERB1_MIN  1
#define MCB_LOGVERB2_NORM 2
#define MCB_LOGVERB3_MED  3
#define MCB_LOGVERB4_HIGH 4
#define MCB_LOGVERB5_ALL  5

// log transaction info is always at the beginning of line and is in this format: "[ESC][CTL]########" 
#define MCB_LOGTRANS_CHAR_VOID '$'     // this character should be treated as no-print void
#define MCB_LOGTRANS_CHAR_ESC  '@'     // this character starts the transaction info
#define MCB_LOGTRANS_CHAR_CTL_STAT 's' // type of the information

// 'i' info flags
#define MCB_LOGTRANS_STAT_OK   0x00000001UL  // record indicating normal behavior
#define MCB_LOGTRANS_STAT_ERR  0x00000002UL  // error result, error occured during processing
#define MCB_LOGTRANS_STAT_WARN 0x00000004UL  // warning result, suspicious behavior etc.
#define MCB_LOGTRANS_STAT_ALL  0x0000ffffUL  // all supported info attributes

#ifdef __cplusplus
} // extern "C"

// class which automates the LOG record creation (in constructor) and termination (in destructor)
class MCBCOM_API CMcbLogger
{
protected:
	HMCBCOM m_hCom;
	HRESULT* m_pHR;
	BOOL* m_pBool;
	VARIANT_BOOL* m_pVBool;

public:
	BOOL IsLogging();

protected:
	void CommonContruct(HMCBCOM hCom);

public:
	CMcbLogger(HMCBCOM hCom, DWORD dwService, int nVerbosity, HRESULT* pRetCode, LPCTSTR pszText, ...);
	CMcbLogger(HMCBCOM hCom, DWORD dwService, int nVerbosity, BOOL* pRetOk, LPCTSTR pszText, ...);
	CMcbLogger(HMCBCOM hCom, DWORD dwService, int nVerbosity, VARIANT_BOOL* pRetOk, LPCTSTR pszText, ...);
	~CMcbLogger(void);
};
#endif /* __cplusplus */

#endif
