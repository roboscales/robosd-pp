#define PREFIX(x) _PREFIX(CLCH_NAME,x)
#define _PREFIX(p,x) __PREFIX(p,x)
#define __PREFIX(p,x) p##_##x
