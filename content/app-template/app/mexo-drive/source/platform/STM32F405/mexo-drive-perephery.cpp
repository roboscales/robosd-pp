#include "mexo-drive.hpp"
#include "core/robosd_system.hpp"
#include <cstdlib>
#include <cstdio>
#include "main.h"
#include "tim.h"
#include "adc.h"
#include "can.h"
//#include "spi.h"
#include "net/robosd_serial.hpp"
#include "net/robosd_flow.hpp"
#include "net/robosd_flow_id.h"
#include "freemaster/robosd_fm.hpp"
#include "prf/as5048a/as5048a.hpp"
#include "terminal/robosd_termo.hpp"
#include "core/robosd_log.hpp"
namespace robo {

	#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_SPECIFIC

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
uint32_t mexo_drive::current_adc_driver::sence[2]={0,0};

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
	mexo_drive::current_adc_driver::sence[0] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1); // читаем полученное значение в переменную adc	
	mexo_drive::current_adc_driver::sence[1] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2); // читаем полученное значение в переменную adc
	HAL_ADCEx_InjectedStart(&hadc1); // запускаем преобразование сигнала ј÷ѕ
	tick_();
	::mexo::machine::priority_loop();
	::mexo::machine::backend_loop();
};
namespace mexo_drive {
void pwm_driver::boot_complete(types::discret_t _duty){
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_GPIO_WritePin(nSD_GPIO_Port,nSD_Pin,GPIO_PIN_SET);
}

void pwm_driver::shutdown_begin(void){
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	HAL_GPIO_WritePin(nSD_GPIO_Port,nSD_Pin,GPIO_PIN_RESET);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
}

void pwm_driver::do_run(types::discret_t _duty){
	if(_duty>0){
		TIM1->CCR1 = (uint32_t)_duty;
		TIM1->CCR2 = 0;
	} else {
		TIM1->CCR1 = 0;
		TIM1->CCR2 = (uint32_t)(-_duty);
	}
}

#define MAX_PWM ADC_START_TIME
perephery_config_s perephery_config=
{
	{ 
		{
			{0}
			, 231
			, 10
		}
		,{
			{
				-MAX_PWM
				,MAX_PWM
			}
			,{
				-32767
				,32767
			}
		} 
	}
	,{
		{0} 
		,{0,1} //index
		,{1, 1} //scale
		,10	// init_count_shift -1024 точек
	}
	,{
		{0}
		, 1
		, 2
	}
	,{
		{0}
		, -1
		, 0
	}
};

void  flow_set_addr( uint8_t _addr){
	//volatile uint32_t tmp = rdk_store_array[0];
	if (_addr > 0 && _addr < 16){
		CAN_FilterTypeDef canFilterConfig={0};
		for(int i=0;i<13;i++){
			canFilterConfig.FilterActivation = CAN_FILTER_DISABLE;
			canFilterConfig.FilterBank = i;
			HAL_CAN_ConfigFilter(&hcan1, &canFilterConfig);
		}
		canFilterConfig.FilterMode = 	CAN_FILTERMODE_IDMASK;
		canFilterConfig.FilterScale =  CAN_FILTERSCALE_32BIT;

		canFilterConfig.FilterMaskIdHigh = 0xDF0<<5;
		canFilterConfig.FilterMaskIdLow = 0;

		canFilterConfig.FilterIdHigh = _addr<<4<<5;
		canFilterConfig.FilterIdLow =  0;
		canFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
		canFilterConfig.FilterBank = 14;

		
		HAL_CAN_ConfigFilter(&hcan1, &canFilterConfig);


		canFilterConfig.FilterMode = 	CAN_FILTERMODE_IDMASK;
		canFilterConfig.FilterScale =  CAN_FILTERSCALE_32BIT;

		canFilterConfig.FilterMaskIdHigh = 0xDF0<<5;
		canFilterConfig.FilterMaskIdLow = 0;

		canFilterConfig.FilterIdHigh = 0x0<<4<<5;
		canFilterConfig.FilterIdLow =  0;
		canFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
		canFilterConfig.FilterBank = 15;
		HAL_CAN_ConfigFilter(&hcan1, &canFilterConfig);
	}
}

class can_port_driver{
	public:
	static inline cstr path = can0_PATH;
	typedef flow_msg_can_id_t id_t;
	enum{suba_count = 16,  packet_size = 8, msg_pool_size = 4 };
	static void send(unsigned _id , const uint8_t * _data, size_t _size);
};

typedef ::robo::net::flow::port_t<can_port_driver> can0_t;
can0_t can0;
void can_port_driver:: send(unsigned _id , const uint8_t * _data, size_t _size){
	if(_size>0){
		CAN_TxHeaderTypeDef header;
		header.DLC =_size;
		header.ExtId = 0;
		header.IDE = CAN_ID_STD;
		header.RTR = CAN_RTR_DATA;
		header.StdId = _id;
		HAL_CAN_AddTxMessage(&hcan1, &header, ( uint8_t *)_data,  (uint32_t *)CAN_TX_MAILBOX0);
	}
}

static volatile uint16_t enco_native_ =0;

void as5048_driver::cs_low(void){
//		HAL_GPIO_WritePin(CSE2_GPIO_Port,CSE2_Pin,GPIO_PIN_RESET);
}
void as5048_driver::cs_hi(void){
//		HAL_GPIO_WritePin(CSE2_GPIO_Port,CSE2_Pin,GPIO_PIN_SET);
}
void as5048_driver::put(uint16_t & _commamd){
//		HAL_SPI_Transmit_IT( &hspi1,(uint8_t *)&_commamd, 2);
}
void as5048_driver::get(uint16_t & _answer){
//		HAL_SPI_Receive_IT( &hspi1, (uint8_t *)&_answer, 2);
}

uint16_t as5040_driver::native(void){
	return htim3.Instance->CNT;
}

}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
 static int trig = 0;
 static CAN_RxHeaderTypeDef RxHeader;
 static uint8_t RxData[8];
HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData);
	mexo_drive::can0.on_receive(RxHeader.StdId,RxData,RxHeader.DLC);

}





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
				HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
				mexo_drive::flow_set_addr(0xA);			
				HAL_CAN_Start(&hcan1);
		  	HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_ALL);
			}
		);
			::mexo::machine::slot::simple enco_put(
		0
		,	&mexo_drive::AS5048A::put
	);

	machine::slot::simple enco_get(
		2
		,	&mexo_drive::AS5048A::get
	);

/*	machine::slot::simple frontend_test_pool_ (
	machine::slot::kind::frontend
	,	[]{
		static volatile robo::time_us_t g_time_us_t2 = 0;
		robo::time_us_t tmp  = robo::system::env::realtime_us();
		if(tmp - g_time_us_t2 > 1000000){
			robo_infolog("aaaaa");
			g_time_us_t2 = tmp;
		}
	}
	);*/
}

