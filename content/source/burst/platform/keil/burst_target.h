#if (!defined(burst_target_h)) && defined(burst_h)
#define burst_target_h
#else
#error error of using burst_target.h
#endif

#define BURST_PROC_NAME __FUNCTION__
#define BURST_PROC_FILE __FILE__
#define BURST_PROC_LINE __LINE__
#define BURST_UNUSED(x) ((void)(x))
#define BURST_EMPTY_STRUCT {0}
#define BURST_WEAK __weak
#define BURST_STATIC_INLINE static inline
