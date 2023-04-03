#include "mexo++drive.hpp"
#include "usart.h"
namespace mppd{
	namespace periphery{
		//пауза мкс
		volatile unsigned int *DWT_CYCCNT = (volatile unsigned int *)0xE0001004; //address of the register
		volatile unsigned int *DWT_CONTROL = (volatile unsigned int *)0xE0001000; //address of the register
		volatile unsigned int *SCB_DEMCR = (volatile unsigned int *)0xE000EDFC; //address of the register 

		clock::tick_t clock::tick(void){
			return *DWT_CYCCNT;
		}
		
		clock::tick_t clock::ns2tick(ns_t _ns){
			return (tick_t)(( (uint64_t)536870912 + (uint64_t)(MPPD_CORE_CLOCK_HZ*1.073741824) *_ns )>>30);
		}
		
		clock::ns_t clock::tick2ns(tick_t _tick){
			return ((uint64_t)((uint64_t)1024*1e9 / MPPD_CORE_CLOCK_HZ) *_tick +512) >> 10;
		}
		clock::tick_t clock::us2tick(us_t _us){
			return (tick_t)(( (uint64_t)524288 + (uint64_t)(MPPD_CORE_CLOCK_HZ*1.048576) *_us )>>20);
		}
		
		clock::ns_t clock::tick2us(tick_t _tick){
			return (uint64_t)((uint64_t)((uint64_t)1024*1024*1e6 / MPPD_CORE_CLOCK_HZ) *_tick +524288) >> 20;
		}
		void clock::idle(void){
			asm("nop");
		}
		slot::simple start( slot::kind::begin,
		[]{
			*SCB_DEMCR = *SCB_DEMCR | 0x01000000; 
			*DWT_CYCCNT = 0; // reset the counter 
			*DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter	
		}
		);
		
	
	}
}
#if ROBO_APP_MEXO_DEBUG_TP1_ENABLED == 1
void mexo::tp_driver::on(void){
	TP1_GPIO_Port->BSRR = TP1_Pin;	
}
void mexo::tp_driver::off(void){
	TP1_GPIO_Port->BSRR = TP1_Pin<< 16;	
}
#endif

namespace robo {

	#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_SPECIFIC

	void  system::env::abort(void) {
		//todo что делаем? надо подумать 
		::mexo::machine::raise_fault();
		::abort();
	}
	
	bool cr_ = false;
	system::guard::op system::env::critical_enter(void) {
		if(!cr_){
			HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
			cr_ = true;
			return system::guard::op::enter;
		} else {
			return system::guard::op::skip;
		}
	}

	void system::env::critical_leave(void) {
		HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
		cr_ = false;
	}
	
	system::context current_context_ = system::context::frontend;
	int locked_ = 0;
	bool system::env::is_frontend(void) {
		return current_context_==context::frontend;
	}

	bool system::env::is_backend(void) {
		return current_context_==context::backend;
	}
	
	system::guard::op system::env::enter(void) {
		uint32_t prim = __get_PRIMASK();
		__disable_irq();
		return prim == 0? system::guard::op::enter : system::guard::op::skip;
	}

	void system::env::leave(void) {
			__enable_irq();
	}
	

	system::guard::op system::env::lock(void) {
		if(locked_ == false){
			locked_ = true;
			return system::guard::op::enter;
		} else {
			return system::guard::op::skip;
		}
	}
	void system::env::unlock(void) {
		ROBO_APP_ASSERT(locked_ == true);
		locked_ = false;
	}
	
	void system::env::fall(void) {
		ROBO_APP_ASSERT(current_context_==context::frontend);
		current_context_=context::backend;
	}
	
	void system::env::comeback(void) {
		ROBO_APP_ASSERT(current_context_==context::backend);
		current_context_=context::frontend;
	}
	
	random_t system::env::rand(random_t _max) {
		return SysTick->VAL % _max;
	}
	
	void system::env::sleep(void) {
		asm("nop");
	}
	
#if ROBO_APP_PRINT_TYPE == ROBO_APP_TYPE_SPECIFIC	
	void system::env::print( cstr  _s){
	#if ROBO_APP_TERMINAL_ENABLED == 1
		if(system::env::is_frontend() )
			::robo::termo::itf::prints(_s);
	#endif
	}
#endif

#if ROBO_APP_ALLOC_TYPE == 	ROBO_APP_TYPE_SPECIFIC
	void* system::env::mem_alloc(size_t _size) {
		return malloc(_size);
	}
	void system::env::mem_free(void * _memo) {
		free(_memo);
	}
#endif

#if ROBO_APP_FORMATING_TYPE == ROBO_APP_TYPE_SPECIFIC
	size_t system::env::sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args) {

	#if ROBO_UNICODE_ENABLED == 1
		size_t sz = vswprintf(_dst, _max_sz, _format, _args);
	#else
		size_t sz = vsnprintf(_dst, _max_sz-1, _format, _args);
	#endif
		if (sz < _max_sz-1) {
			_dst[sz] = 0;
		}
		return sz;
	}
#endif
#endif





}