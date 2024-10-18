#ifndef burst_h
#define burst_h

#if defined(__cplusplus)
extern "C"
{
#endif


void burst_begin(void);
void burst_begin_ps(unsigned int _period_us);
void burst_realtime_loop(void);
void burst_backend_loop(void);
void burst_frontend_loop(void);
void burst_core_crash(void);

#if defined(__cplusplus)
}
#endif
				
				
#endif