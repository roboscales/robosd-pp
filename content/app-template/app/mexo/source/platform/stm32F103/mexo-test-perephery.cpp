#include "core/robosd_system.hpp"
#include <cstdlib>
#include <cstdio>
#include "main.h"
#include "tim.h"
#include "mexo-test.hpp"

namespace robo {

	#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_KEIL

	void  system::env::abort(void) {
		::abort();
	}
	
	static time_us_t period_us_ = 50;
	static time_us_t us_ = 0;
	static time_ms_t ms_ = 0;

	void tick_(void) {
		static time_us_t us_acc_ = 0;
		us_+=period_us_;
		us_acc_+=period_us_;
		if(us_acc_>=1000){
			us_acc_-=1000;
			ms_++;
		}
	}

	void * system::env::critical_enter(void) {
		return nullptr;
	}
	
	void system::env::critical_leave(void * /*context_*/) {
	}
	
	system::context current_context_ = system::context::frontend;
	int locked_ = 0;
	bool system::env::is_frontend(void) {
		return current_context_==context::frontend;
	}

	bool system::env::is_backend(void) {
		return current_context_==context::backend;
	}
	
	void* system::env::enter(void) {
		uint32_t prim = __get_PRIMASK();
		__disable_irq();
		return (void *) prim;
		
	}
	
	void system::env::leave(void* _context) {
		int32_t prim = (uint32_t)_context;
		if(!prim) 
			__enable_irq();
	}
	

	void system::env::lock(void) {
		__disable_irq();
		ROBO_APP_ASSERT(locked_ == false);			
		locked_ = true;
	}
	void system::env::unlock(void) {
		ROBO_APP_ASSERT(locked_ == true);			
		locked_ = false;
		__enable_irq();
	}
	
	void system::env::fall(void) {
		ROBO_APP_ASSERT(current_context_==context::frontend);
		current_context_=context::backend;
	}
	
	void system::env::comeback(void) {
		ROBO_APP_ASSERT(current_context_==context::backend);
		current_context_=context::frontend;
	}
	
	time_us_t system::env::time_us(void) {
		return us_;
	}
	
	time_us_t system::env::realtime_us(void) {
		return us_;
	}
	
	time_ms_t system::env::time_ms(void) {			
		return ms_;
	}
	
	random_t system::env::rand(random_t _max) {
		return SysTick->VAL % _max;
	}
	
	void system::env::wakeup(void) {
	}
	
	time_us_t system::env::period_us(void) {
		return period_us_;
	}
	
	void system::env::sleep(void) {
	}
	
	void system::env::print(robo::log::verb _verb, cstr _format, va_list  _args){
	}
	void system::env::print( cstr  _s){
/*		
		transport_buffer * packet = transport_buffer_query();
		while (packet)	{
			uint8_t * ptr = packet->memo;
			while(*_s && packet->space > 0){
				*ptr++ = (uint8_t)*_s++;
				packet->space--;
			}
			if( packet->space == 0){
				transport_buffer_push(packet);
				if(*_s==0){
					return;
				}
			}
			else{
				if(*_s==0){
					transport_buffer_push(packet);
					return;
				}
			}
			packet = transport_buffer_query();						
		}	
	*/	
		/*while(*_s){
			huart3.Instance->DR = *_s++;
			while ( (huart3.Instance->SR & (UART_FLAG_TC)) != (UART_FLAG_TC) );
		}*/
		
	}
	
#if ROBO_APP_ALLOC_ENABLED == 1
	void* system::env::mem_alloc(size_t _size) {
		return malloc(_size);
	}
	void system::env::mem_free(void * _memo) {
		free(_memo);
	}
#endif

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
}
#include "mexo/mexo.hpp"
namespace mexo{
	void tp_driver::on(void){
		HAL_GPIO_WritePin(TP1_GPIO_Port,TP1_Pin, GPIO_PIN_SET);
	}
	void tp_driver::off(void){
		HAL_GPIO_WritePin(TP1_GPIO_Port,TP1_Pin, GPIO_PIN_RESET);
	}
	
	machine::slot::simple start(
		machine::slot::kind::start
		, [] {
				HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_3);	
		}
	);

}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
	tick_();
	::mexo::machine::priority_loop();
	::mexo::machine::backend_loop();
};

void pwm::boot_complete(actual_t _duty){
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
}
void pwm::shutdown_begin(void){
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
}

void pwm::do_run(actual_t _duty){
	if(_duty>0){
		TIM1->CCR1 = (uint32_t)_duty;
		TIM1->CCR2 = 0;
	} else {
		TIM1->CCR1 = 0;
		TIM1->CCR2 = (uint32_t)(-_duty);
	}
}

dc::config_s dc_config = {
	{0} //block
	,{ //converter
		-MAX_PWM
		, MAX_PWM
		, (::mexo::signal_t)MAX_PWM/(::mexo::signal_t)12
	}
};