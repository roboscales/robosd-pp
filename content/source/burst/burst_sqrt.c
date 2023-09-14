#include "burst_sqrt.h"

#define DSQRT_MAX_BIT 24
#define DSQRT_SIZE 24*4
#define DSQRT_MAX_X 16777215
#define BIT_SIZE uint8_t
 uint32_t dsqrtGainArray[22] =
 { 128, 128, 128, 128, 96, 80, 112, 72, 104, 76, 108, 74, 106, 75, 106, 75,
  106, 75, 106, 75, 106, 75};
  
uint32_t dsqrtOffsetArray[22] =
{1, 2, 2, 4, 5, 6, 9, 14, 19, 26, 37, 54, 76, 107, 152, 214, 304, 429, 608, 859, 
1216, 1719 };

uint32_t dsqrtShift[22] = {
  9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 
  19, 19 };

uint32_t dsqrtRes[4] ={0,1,1,2};

uint32_t  dsqrtBitTable[256];


static inline BIT_SIZE dsqrt_bit_bsm(uint32_t n){
   BIT_SIZE res = 0;
  while (n != 1) {
    n >>= 1;
    res++;
  }
  return res;
}
/*
static inline BIT_SIZE dsqrt_bit_bsm2(uint32_t x){
    uint32_t y;
    y = (x >> 24);     if ( y!= 0)  return dsqrtBitTable[y]+24; 
    y = (x >> 16);    if (y != 0)  return dsqrtBitTable[y]+16; 
    y = (x >> 8);  if (y  != 0)  return dsqrtBitTable [y]+8;  
    return dsqrtBitTable[x];
}
*/
uint32_t burst_sqrt( uint32_t  x){
  if(x>DSQRT_MAX_X)
    x = DSQRT_MAX_X;
  if (x<4){
    return dsqrtRes[x];
  }else{
    BIT_SIZE ix = dsqrt_bit_bsm(x)-2;
    return ((x*dsqrtGainArray[ix])>>dsqrtShift[ix]) + dsqrtOffsetArray[ix] ;
  }
}

void burst_sqrt_init(void){
  dsqrtBitTable[0] =  0;
	uint8_t n;
  for (n=255;n>0;n--){
    dsqrtBitTable[n]   = dsqrt_bit_bsm(n);
  }
}

