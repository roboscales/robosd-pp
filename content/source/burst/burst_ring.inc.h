#include "burst/burst.h"
#ifndef RING_PREFIX_NAME 
#define RING_PREFIX_NAME default_ring 
#endif 

#define RING_PREFIX(name)  _RING_PREFIX(name,RING_PREFIX_NAME)
#define _RING_PREFIX(name,prfx)  __RING_PREFIX(name,prfx)
#define __RING_PREFIX(name,prfx) prfx##_##name

#ifndef RING_SIZE_BITS 
#define RING_SIZE_BITS 8
#endif 

#ifndef RING_DATA_T
#define RING_DATA_T unsigned char
#endif

#ifndef RING_LOCK
#define RING_LOCK()
#endif

#ifndef RING_UNLOCK
#define RING_UNLOCK()
#endif


#ifndef RING_SIZE_T
#define RING_SIZE_T unsigned int
#endif

 // маска для индексов
#define RING_SIZE (1 << RING_SIZE_BITS)

#define RING_MASK (RING_SIZE-1)


RING_DATA_T RING_PREFIX(data)[RING_SIZE];    
// количество чтений
RING_SIZE_T RING_PREFIX(readCount);
// количество записей
RING_SIZE_T RING_PREFIX(writeCount);


#ifndef BURST_STATIC_INLINE
#define BURST_STATIC_INLINE
#endif

BURST_STATIC_INLINE void RING_PREFIX(put_)(RING_DATA_T  _value)
{
	RING_PREFIX(data)[ RING_PREFIX(writeCount) & RING_MASK] = _value;
	RING_PREFIX(writeCount)++;
}

// чтение из буфера, возвращает текущий символ
BURST_STATIC_INLINE RING_DATA_T RING_PREFIX(get_)()
{
	return RING_PREFIX(data)[ RING_PREFIX(readCount)++ & RING_MASK];
}

// пуст ли буфер
BURST_STATIC_INLINE burst_bool_t RING_PREFIX(available_)()
{
	return RING_PREFIX(writeCount) != RING_PREFIX(readCount) ? burst_true : burst_false;
}


// количество элементов в буфере
BURST_STATIC_INLINE RING_SIZE_T RING_PREFIX(count_)()
{
	return ( RING_PREFIX(writeCount) - RING_PREFIX(readCount)) & ((RING_MASK<<1)+1); //как это раньше работало?
}

// полон ли буфер
BURST_STATIC_INLINE burst_bool_t RING_PREFIX(full_)()
{
	return ((RING_SIZE_T)(RING_PREFIX(writeCount) - RING_PREFIX(readCount)) & (RING_SIZE_T)~(RING_MASK)) != 0 ? burst_true : burst_false;
}

// очистить буфер
BURST_STATIC_INLINE void RING_PREFIX(clear_)()
{
	RING_PREFIX(readCount) = 0;
	RING_PREFIX(writeCount) = 0;
}

BURST_STATIC_INLINE void RING_PREFIX(buf_put_)(RING_DATA_T * _buf, RING_SIZE_T _len){
	while( _len-- ){
		RING_PREFIX(put_)(*_buf++);
	}
}


BURST_STATIC_INLINE void RING_PREFIX(buf_get_)(RING_DATA_T * _buf, RING_SIZE_T _len){
    while(_len--){
        *_buf++ = RING_PREFIX(get_)();
    }
}


#if defined(RING_LOCK) && defined(RING_UNLOCK)
// количество элементов в буфере
BURST_STATIC_INLINE RING_SIZE_T RING_PREFIX(count)()
{
	RING_SIZE_T ret;
	RING_LOCK();
	ret = RING_PREFIX(count_)();
	RING_UNLOCK();
	return ret;
}

// очистить буфер
BURST_STATIC_INLINE void RING_PREFIX(clear)()
{
	RING_LOCK();
	RING_PREFIX(clear_)();
	RING_UNLOCK();
}




BURST_STATIC_INLINE burst_bool_t RING_PREFIX(buf_put)(RING_DATA_T * _buf, RING_SIZE_T _len){
	RING_LOCK();
	if( RING_PREFIX(count_)()+_len >RING_SIZE){
		RING_UNLOCK();
		return burst_false;
	}else{
		while( _len-- ){
			RING_PREFIX(put_)(*_buf++);
		}
		RING_UNLOCK();
		return burst_true;
	}
}


BURST_STATIC_INLINE RING_SIZE_T RING_PREFIX(buf_get)(RING_DATA_T * _buf, RING_SIZE_T max_len){
	RING_SIZE_T av;
	RING_SIZE_T ret;
	RING_LOCK();
	av = RING_PREFIX(count)();
	if(av>max_len){
			av = max_len;
	}
	ret = av;
	while(av--){
			*_buf++ = RING_PREFIX(get_)();
	}
	RING_UNLOCK();
	return ret;
}

#endif

#undef  RING_PREFIX_NAME 
#undef  RING_PREFIX 
#undef  _RING_PREFIX 
#undef  __RING_PREFIX 

#undef  RING_SIZE_BITS 
#undef  RING_DATA_T 

#undef   RING_LOCK
#undef   RING_UNLOCK
#undef   RING_SIZE_T
#undef   RING_SIZE
#undef   RING_MASK


