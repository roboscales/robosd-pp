#include "mexo++drive.hpp"
#include "core/robosd_system.hpp"
#include <cstdlib>
#include <cstdio>
#include "main.h"
#include "core/robosd_log.hpp"
namespace mppd{
	namespace prf{
		//пауза мкс
		volatile unsigned int *DWT_CYCCNT = (volatile unsigned int *)0xE0001004; //address of the register
		volatile unsigned int *DWT_CONTROL = (volatile unsigned int *)0xE0001000; //address of the register
		volatile unsigned int *SCB_DEMCR = (volatile unsigned int *)0xE000EDFC; //address of the register 

		uint32_t clock::tick(void){
			return *DWT_CYCCNT;
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
namespace robo {

	#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_SPECIFIC

	bool cr_ = false;
	system::guard::op system::env::critical_enter(void) {
		if(!cr_){
			//HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
			cr_ = true;
			return system::guard::op::enter;
		} else {
			return system::guard::op::skip;
		}
	}

	void system::env::critical_leave(void) {
		//HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
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
	
	time_us_t system::env::realtime_us(void) {
		return time_us();
	}
	
	random_t system::env::rand(random_t _max) {
		return SysTick->VAL % _max;
	}
	
	void system::env::wakeup(void) {
	}

	
	void system::env::sleep(void) {
		asm("nop");
	}
	
	void system::env::abort(){
		Error_Handler();
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
#include "mexo/mexo.hpp"

namespace mexo{
	#if MPPD_MEXO_TP1_ENABLED == 1
	void tp_driver::on(void){
		HAL_GPIO_WritePin(TP1_GPIO_Port,TP1_Pin, GPIO_PIN_SET);
	}
	void tp_driver::off(void){
		HAL_GPIO_WritePin(TP1_GPIO_Port,TP1_Pin, GPIO_PIN_RESET);
	}
	#endif
}

