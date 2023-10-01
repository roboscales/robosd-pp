#ifndef clch_pca95xx_h
#define clch_pca95xx_h

#endif
#if defined(CLCH_NAME) 

#include "burst/cliche/_begin.h"

#ifdef CLCH_HEADER

#include "stdint.h"

uint8_t	PREFIX(get)(void);
void	PREFIX(put)(uint8_t data);
void	PREFIX(hw_send)(uint8_t addr, uint8_t data);
uint8_t	PREFIX(hw_receive)(uint8_t addr);

#else


uint8_t	PREFIX(get)(void){
	return PREFIX(hw_receive)(PREFIX(ADDR));
}

void	PREFIX(put)(uint8_t _value){
	PREFIX(hw_send)(PREFIX(ADDR), _value);
}

BURST_WEAK void	PREFIX(hw_send)(uint8_t addr, uint8_t data){
}

BURST_WEAK uint8_t	PREFIX(hw_receive)(uint8_t addr){
	return 0xFF;
}

#endif	

#include "burst/cliche/_end.h"

#endif
