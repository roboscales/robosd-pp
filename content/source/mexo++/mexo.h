#ifndef mexo_h
#define mexo_h

#if defined(__cplusplus)
extern "C"
{
#endif


void mexo_begin(void);
void mexo_begin_ps(unsigned int _period_us);
void mexo_realtime_loop(void);
void mexo_backend_loop(void);
void mexo_frontend_loop(void);
void mexo_raise_fault(void);

#if defined(__cplusplus)
}
#endif
				
				
#endif