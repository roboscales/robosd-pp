#ifndef mexo_h
#define mexo_h

#if defined(__cplusplus)
extern "C"
{
#endif


void mexo_begin(void);
void mexo_start(void);
void mexo_priority_loop(void);
void mexo_backend_loop(void);
void mexo_frontend_loop();

#if defined(__cplusplus)
}
#endif
				
				
#endif