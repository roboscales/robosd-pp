#ifndef __PE_freemaster_delegat
#define __PE_freemaster_delegat
#include "freemaster/PE_freemaster.h"
#include "freemaster/PE_freemaster_private.h"

#if defined(__cplusplus)
extern "C"
{
#endif

void FMSTR_SCI_PUTCHAR(FMSTR_U8 );
FMSTR_U16 FMSTR_SCI_GETCHAR(void);
void FMSTR_SCI_RE(void);
void FMSTR_SCI_RD(void);
void FMSTR_SCI_TE(void);
void FMSTR_SCI_TD(void);
FMSTR_SCISR FMSTR_SCI_RDCLRSR(void);
void freemaster_init(void);
void freemaster_put(FMSTR_U8 * _data, FMSTR_U16 _len );
void freemaster_get(FMSTR_U8 * _data, FMSTR_U16 _len );
FMSTR_U16 freemaster_get_out_bytes_count(void);
FMSTR_U16 freemaster_get_in_bytes_count(void);
#if defined(__cplusplus)
}
#endif

#endif
