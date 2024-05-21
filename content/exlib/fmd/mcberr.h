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
*   This header file defines error codes of the FreeMASTER communication library.
*
*******************************************************************************/

////////////////////////////////////////////
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: MCB_S_OK
//
// MessageText:
//
// No error.
//
#define MCB_S_OK                         ((DWORD)0x00000000L)

//
// MessageId: MCB_S_RECRUN
//
// MessageText:
//
// Recorder is running.
//
#define MCB_S_RECRUN                     ((DWORD)0x00000001L)

//
// MessageId: MCB_S_RECDONE
//
// MessageText:
//
// Recorder data prepared.
//
#define MCB_S_RECDONE                    ((DWORD)0x00000002L)

/////////////////////////////////////////////
// Standard (no-facility) errors - 
// for mcbcom.dll calls only
/////////////////////////////////////////////

//
// MessageId: MCB_E_INVCMD
//
// MessageText:
//
// Invalid command.
//
#define MCB_E_INVCMD                     ((DWORD)0x80000081L)

//
// MessageId: MCB_E_CMDCSERR
//
// MessageText:
//
// Command checksum error.
//
#define MCB_E_CMDCSERR                   ((DWORD)0x80000082L)

//
// MessageId: MCB_E_CMDTOOLONG
//
// MessageText:
//
// Command packet too long.
//
#define MCB_E_CMDTOOLONG                 ((DWORD)0x80000083L)

//
// MessageId: MCB_E_RSPBUFFOVF
//
// MessageText:
//
// Response buffer overflow.
//
#define MCB_E_RSPBUFFOVF                 ((DWORD)0x80000084L)

//
// MessageId: MCB_E_INVBUFF
//
// MessageText:
//
// Invalid buffer specified.
//
#define MCB_E_INVBUFF                    ((DWORD)0x80000085L)

//
// MessageId: MCB_E_INVSIZE
//
// MessageText:
//
// Invalid size specified.
//
#define MCB_E_INVSIZE                    ((DWORD)0x80000086L)

//
// MessageId: MCB_E_SERVBUSY
//
// MessageText:
//
// Service is busy.
//
#define MCB_E_SERVBUSY                   ((DWORD)0x80000087L)

//
// MessageId: MCB_E_NOTINIT
//
// MessageText:
//
// Service is not initialised.
//
#define MCB_E_NOTINIT                    ((DWORD)0x80000088L)

//
// MessageId: MCB_E_EACCESS
//
// MessageText:
//
// Access to target resource is denied.
//
#define MCB_E_EACCESS                    ((DWORD)0x80000089L)

//
// MessageId: MCB_E_SFIOERR
//
// MessageText:
//
// Error returned by SFIO system.
//
#define MCB_E_SFIOERR                    ((DWORD)0x8000008AL)

//
// MessageId: MCB_E_SFIOUNMATCH
//
// MessageText:
//
// Mismatch in SFIO request/response frame sequence. 
//
#define MCB_E_SFIOUNMATCH                ((DWORD)0x8000008BL)

//
// MessageId: MCB_E_PIPEERR
//
// MessageText:
//
// Pipe access error (pipe not open on target?). 
//
#define MCB_E_PIPEERR                    ((DWORD)0x8000008CL)

//
// MessageId: MCB_E_FASTRECERR
//
// MessageText:
//
// Fast recorder initialization error (trying to change fixed settings?). 
//
#define MCB_E_FASTRECERR                 ((DWORD)0x8000008DL)

//
// MessageId: MCB_E_CANTGLERR
//
// MessageText:
//
// CAN fragmentation (toggle bit) error. 
//
#define MCB_E_CANTGLERR                  ((DWORD)0x8000008EL)

//
// MessageId: MCB_E_CANMSGERR
//
// MessageText:
//
// CAN message format error. 
//
#define MCB_E_CANMSGERR                  ((DWORD)0x8000008FL)

//
// MessageId: MCB_E_FLOATDISABLED
//
// MessageText:
//
// Floating point triggering is not enabled. 
//
#define MCB_E_FLOATDISABLED              ((DWORD)0x80000090L)

//
// MessageId: MCB_E_UNKNOWN
//
// MessageText:
//
// Unknown board error.
//
#define MCB_E_UNKNOWN                    ((DWORD)0x800000FFL)

//
// MessageId: MCB_E_CANTWRITE
//
// MessageText:
//
// Serial port write error.
//
#define MCB_E_CANTWRITE                  ((DWORD)0x80000100L)

//
// MessageId: MCB_E_RESPTMOUT
//
// MessageText:
//
// Response timeout.
//
#define MCB_E_RESPTMOUT                  ((DWORD)0x80000101L)

//
// MessageId: MCB_E_RESPCSERR
//
// MessageText:
//
// Response checksum error.
//
#define MCB_E_RESPCSERR                  ((DWORD)0x80000102L)

//
// MessageId: MCB_E_CANTDETECT
//
// MessageText:
//
// Can't detect board.
//
#define MCB_E_CANTDETECT                 ((DWORD)0x80000103L)

//
// MessageId: MCB_E_BUFFSMALL
//
// MessageText:
//
// Buffer size too small.
//
#define MCB_E_BUFFSMALL                  ((DWORD)0x80000104L)

//
// MessageId: MCB_E_INVPOINTER
//
// MessageText:
//
// Buffer pointer invalid.
//
#define MCB_E_INVPOINTER                 ((DWORD)0x80000105L)

//
// MessageId: MCB_E_LIMITEXCDED
//
// MessageText:
//
// Limit for number of entities exceeded.
//
#define MCB_E_LIMITEXCDED                ((DWORD)0x80000106L)

//
// MessageId: MCB_E_A32NOTSUPP
//
// MessageText:
//
// 32-bit adresses not supported.
//
#define MCB_E_A32NOTSUPP                 ((DWORD)0x80000107L)

//
// MessageId: MCB_E_D32NOTSUPP
//
// MessageText:
//
// 32-bit data values not supported.
//
#define MCB_E_D32NOTSUPP                 ((DWORD)0x80000108L)

//
// MessageId: MCB_E_ERRSIZE
//
// MessageText:
//
// Error in size.
//
#define MCB_E_ERRSIZE                    ((DWORD)0x80000109L)

//
// MessageId: MCB_E_ERRSIZEBUS
//
// MessageText:
//
// Size incompatible with bus width.
//
#define MCB_E_ERRSIZEBUS                 ((DWORD)0x8000010AL)

//
// MessageId: MCB_E_ERRDATA
//
// MessageText:
//
// Error in entry data.
//
#define MCB_E_ERRDATA                    ((DWORD)0x8000010BL)

//
// MessageId: MCB_E_RESPWRONG
//
// MessageText:
//
// Error in response.
//
#define MCB_E_RESPWRONG                  ((DWORD)0x8000010CL)

//
// MessageId: MCB_E_PIPENOTOPEN
//
// MessageText:
//
// The pipe requested is not open.
//
#define MCB_E_PIPENOTOPEN                ((DWORD)0x8000010DL)

//
// MessageId: MCB_E_PIPEEXISTS
//
// MessageText:
//
// This pipe is already open.
//
#define MCB_E_PIPEEXISTS                 ((DWORD)0x8000010EL)

//
// MessageId: MCB_E_NOMOREPIPES
//
// MessageText:
//
// Maximum number of open pipes reached.
//
#define MCB_E_NOMOREPIPES                ((DWORD)0x8000010FL)

//
// MessageId: MCB_E_LOGEMPTY
//
// MessageText:
//
// No more entries in the log.
//
#define MCB_E_LOGEMPTY                   ((DWORD)0x80000110L)

//
// MessageId: MCB_E_FPROGERROR
//
// MessageText:
//
// Flash Programmer - No error.
//
#define MCB_E_FPROGERROR                 ((DWORD)0x80000200L)

//
// MessageId: MCB_E_FPROGERROR_FAIL
//
// MessageText:
//
// Flash Programmer: Operation failed.
//
#define MCB_E_FPROGERROR_FAIL            ((DWORD)0x80000201L)

//
// MessageId: MCB_E_FPROGERROR_NOACC
//
// MessageText:
//
// Flash Programmer: Access denied.
//
#define MCB_E_FPROGERROR_NOACC           ((DWORD)0x80000202L)

//
// MessageId: MCB_E_FPROGERROR_INVPRM
//
// MessageText:
//
// Flash Programmer: Invalid parameters.
//
#define MCB_E_FPROGERROR_INVPRM          ((DWORD)0x80000203L)

//
// MessageId: MCB_E_FPROGERROR_MISSALIGN
//
// MessageText:
//
// Flash Programmer: Misaligned access not possible (size and address must be rounded).
//
#define MCB_E_FPROGERROR_MISSALIGN       ((DWORD)0x80000204L)

//
// MessageId: MCB_E_FPROGERROR_205
//
// MessageText:
//
// Flash Programmer error 5.
//
#define MCB_E_FPROGERROR_205             ((DWORD)0x80000205L)

//
// MessageId: MCB_E_FPROGERROR_206
//
// MessageText:
//
// Flash Programmer error 6.
//
#define MCB_E_FPROGERROR_206             ((DWORD)0x80000206L)

//
// MessageId: MCB_E_FPROGERROR_207
//
// MessageText:
//
// Flash Programmer error 7.
//
#define MCB_E_FPROGERROR_207             ((DWORD)0x80000207L)

//
// MessageId: MCB_E_FPROGERROR_208
//
// MessageText:
//
// Flash Programmer error 8.
//
#define MCB_E_FPROGERROR_208             ((DWORD)0x80000208L)

//
// MessageId: MCB_E_FPROGERROR_209
//
// MessageText:
//
// Flash Programmer error 9.
//
#define MCB_E_FPROGERROR_209             ((DWORD)0x80000209L)

//
// MessageId: MCB_E_FPROGERROR_20A
//
// MessageText:
//
// Flash Programmer error 10.
//
#define MCB_E_FPROGERROR_20A             ((DWORD)0x8000020AL)

//
// MessageId: MCB_E_FPROGERROR_20B
//
// MessageText:
//
// Flash Programmer error 11.
//
#define MCB_E_FPROGERROR_20B             ((DWORD)0x8000020BL)

//
// MessageId: MCB_E_FPROGERROR_20C
//
// MessageText:
//
// Flash Programmer error 12.
//
#define MCB_E_FPROGERROR_20C             ((DWORD)0x8000020CL)

//
// MessageId: MCB_E_FPROGERROR_20D
//
// MessageText:
//
// Flash Programmer error 13.
//
#define MCB_E_FPROGERROR_20D             ((DWORD)0x8000020DL)

//
// MessageId: MCB_E_FPROGERROR_20E
//
// MessageText:
//
// Flash Programmer error 14.
//
#define MCB_E_FPROGERROR_20E             ((DWORD)0x8000020EL)

//
// MessageId: MCB_E_FPROGERROR_20F
//
// MessageText:
//
// Flash Programmer error 15.
//
#define MCB_E_FPROGERROR_20F             ((DWORD)0x8000020FL)

//
// MessageId: MCB_E_FPROGINVBSZ
//
// MessageText:
//
// Flash Programmer I/O Buffer size invalid.
//
#define MCB_E_FPROGINVBSZ                ((DWORD)0x80000280L)

//
// MessageId: MCB_E_FPROGINVCSZ
//
// MessageText:
//
// Flash Programmer Command/Status size invalid.
//
#define MCB_E_FPROGINVCSZ                ((DWORD)0x80000281L)

//
// MessageId: MCB_E_FPROGTMOUT
//
// MessageText:
//
// Flash Programmer operation timeout.
//
#define MCB_E_FPROGTMOUT                 ((DWORD)0x80000282L)

//
// MessageId: MCB_E_FPROGNOCFG
//
// MessageText:
//
// Flash Programmer not yet configured.
//
#define MCB_E_FPROGNOCFG                 ((DWORD)0x80000283L)

//
// MessageId: MCB_E_FPROGADDR32BIT
//
// MessageText:
//
// Flash Programmer needs to use 32bit addressing.
//
#define MCB_E_FPROGADDR32BIT             ((DWORD)0x80000284L)

//
// MessageId: MCB_E_FPROGBUSY
//
// MessageText:
//
// Flash Programmer still processing previous command.
//
#define MCB_E_FPROGBUSY                  ((DWORD)0x80000285L)

//
// MessageId: MCB_E_FPROGABORTED
//
// MessageText:
//
// Flash Programmer operation aborted.
//
#define MCB_E_FPROGABORTED               ((DWORD)0x80000286L)

//
// MessageId: MCB_E_SFIOUNEX
//
// MessageText:
//
// Unexpected SFIO system behavior.
//
#define MCB_E_SFIOUNEX                   ((DWORD)0x80000287L)

//
// MessageId: MCB_E_PORTCLOSED
//
// MessageText:
//
// Communication port is not opened.
//
#define MCB_E_PORTCLOSED                 ((DWORD)0x8000FFFBL)

//
// MessageId: MCB_E_OBSOLETED
//
// MessageText:
//
// Library function obsoleted.
//
#define MCB_E_OBSOLETED                  ((DWORD)0x8000FFFCL)

//
// MessageId: MCB_E_TOOMANYTHRDS
//
// MessageText:
//
// Too many threads accessing remote server.
//
#define MCB_E_TOOMANYTHRDS               ((DWORD)0x8000FFFDL)

//
// MessageId: MCB_E_NOTIMPL
//
// MessageText:
//
// Not implemented in current library version.
//
#define MCB_E_NOTIMPL                    ((DWORD)0x8000FFFEL)

//
// MessageId: MCB_E_UNEXPECTED
//
// MessageText:
//
// Unexpected error.
//
#define MCB_E_UNEXPECTED                 ((DWORD)0x8000FFFFL)

/////////////////////////////////////////////
// General MCB plugin errors (own-facility)
/////////////////////////////////////////////

//
// MessageId: MCBPGI_E_CSINV
//
// MessageText:
//
// Connect string invalid.
//
#define MCBPGI_E_CSINV                   ((DWORD)0x848A0001L)

//
// MessageId: MCBPGI_E_NOTOPEN
//
// MessageText:
//
// Port not opened.
//
#define MCBPGI_E_NOTOPEN                 ((DWORD)0x848A0002L)

//
// MessageId: MCBPGI_E_ALREADYOPEN
//
// MessageText:
//
// Port already open.
//
#define MCBPGI_E_ALREADYOPEN             ((DWORD)0x848A0003L)

//
// MessageId: MCBPGI_E_LOCKVIOLATION
//
// MessageText:
//
// The requested feature is locked by another client.
//
#define MCBPGI_E_LOCKVIOLATION           ((DWORD)0x848A0004L)

//
// MessageId: MCBPGI_E_LOCKNEEDED
//
// MessageText:
//
// The feature must be locked before using it.
//
#define MCBPGI_E_LOCKNEEDED              ((DWORD)0x848A0005L)

//
// MessageId: MCBPGI_E_CANTCONNECT
//
// MessageText:
//
// Can not connect to the server.
//
#define MCBPGI_E_CANTCONNECT             ((DWORD)0x848A0006L)

//
// MessageId: MCBPGI_E_SVRRESPBAD
//
// MessageText:
//
// Response from the server was bad.
//
#define MCBPGI_E_SVRRESPBAD              ((DWORD)0x848A0007L)

/////////////////////////////////////////////
// Remote server specific errors
/////////////////////////////////////////////

//
// MessageId: MCBPGI_E_PORTNOTFOUND
//
// MessageText:
//
// Specified communication port not found on the server.
//
#define MCBPGI_E_PORTNOTFOUND            ((DWORD)0x848A1001L)

//
// MessageId: MCBPGI_E_PORTDISABLED
//
// MessageText:
//
// Specified communication port is disabled on the server.
//
#define MCBPGI_E_PORTDISABLED            ((DWORD)0x848A1002L)

//
// MessageId: MCBPGI_E_EXCLOPEN
//
// MessageText:
//
// Can not open port, port is alrady open in exclusive mode.
//
#define MCBPGI_E_EXCLOPEN                ((DWORD)0x848A1003L)

//
// MessageId: MCBPGI_E_CANTEXCL
//
// MessageText:
//
// Port is alrady open, can not open in exclusive mode.
//
#define MCBPGI_E_CANTEXCL                ((DWORD)0x848A1004L)

//
// MessageId: MCBPGI_E_DIFFSPEED
//
// MessageText:
//
// Port is already open and communicates on different speed.
//
#define MCBPGI_E_DIFFSPEED               ((DWORD)0x848A1005L)

//
// MessageId: MCBPGI_E_HTTPBADFNUM
//
// MessageText:
//
// Bad operation requested on HTTP server.
//
#define MCBPGI_E_HTTPBADFNUM             ((DWORD)0x848A1007L)

//
// MessageId: MCBPGI_E_HTTPBADPARAM
//
// MessageText:
//
// Bad parameters passed to HTTP server.
//
#define MCBPGI_E_HTTPBADPARAM            ((DWORD)0x848A1008L)

