#ifdef PWM_BRIDGE_NAME
#define PWM_BRIDGE_PREFIX(x) _PWM_BRIDGE_PREFIX(PWM_BRIDGE_NAME,x)
#define _PWM_BRIDGE_PREFIX(p,x) __PWM_BRIDGE_PREFIX(p,x)
#define __PWM_BRIDGE_PREFIX(p,x) p##_##x

#ifndef PWM_BRIDGE_BOOT_DELAY
#define PWM_BRIDGE_BOOT_DELAY 0
#endif

#ifndef PWM_BRIDGE_SHUTDOWN_DELAY
#define PWM_BRIDGE_SHUTDOWN_DELAY 0
#endif

#ifndef PWM_BRIDGE_BOOT
#define PWM_BRIDGE_BOOT() 
#endif

#ifndef PWM_BRIDGE_ON
#define PWM_BRIDGE_ON()
#endif

#ifndef PWM_BRIDGE_OFF
#define PWM_BRIDGE_OFF()
#endif

#ifndef PWM_BRIDGE_SHUTDOWN
#define PWM_BRIDGE_SHUTDOWN()
#endif

#ifndef PWM_BRIDGE_SET_PWM
PWM_BRIDGE_SET_PWM(x)
#endif

uint16_t PWM_BRIDGE_PREFIX(counter_tick)  = 0;


void PWM_BRIDGE_PREFIX(run)(mexo_dev_p _dev){
	switch (_dev->power_state){
	case MEXO_PS_START:
		//g_board_driver_shared->pwm_bridge.boot();
		PWM_BRIDGE_BOOT();
		PWM_BRIDGE_PREFIX(counter_tick) = PWM_BRIDGE_BOOT_DELAY;
		_dev->power_state = MEXO_PS_BOOT;
	case MEXO_PS_BOOT:
		if (PWM_BRIDGE_PREFIX(counter_tick) > 0){
			PWM_BRIDGE_PREFIX(counter_tick)--;
			break;
		}
		//g_board_driver_shared->pwm_bridge.on();
		PWM_BRIDGE_ON();
		_dev->power_state = MEXO_PS_ON;
	case MEXO_PS_ON:
		if (_dev->power_req_state == MEXO_PS_REQ_ON){		
			static mexo_signal_t  duty = 0;
			duty = s_mult(PWM_BRIDGE_PWM_SOURCE,PWM_BRIDGE_PWM_MAX);
			PWM_BRIDGE_SET_PWM(duty);
			break;
		}
		PWM_BRIDGE_OFF();
		PWM_BRIDGE_PREFIX(counter_tick) = PWM_BRIDGE_SHUTDOWN_DELAY;
		_dev->power_state = MEXO_PS_SHUTDOWN;
	case MEXO_PS_SHUTDOWN:
		if (PWM_BRIDGE_PREFIX(counter_tick) > 0){
			PWM_BRIDGE_PREFIX(counter_tick)--;
		}
		else{
			//g_board_driver_shared->pwm_bridge.shutdown();
			PWM_BRIDGE_SHUTDOWN();
			_dev->power_state = MEXO_PS_OFF;
		}
		break;
	case MEXO_PS_OFF:
		if (_dev->power_req_state == MEXO_PS_REQ_ON){
			_dev->power_state = MEXO_PS_START;
		}
	case MEXO_PS_DISABLE:
		break;
	default:
		break;
	}
}

#undef PWM_BRIDGE_NAME
#undef  PWM_BRIDGE_PREFIX
#undef  _PWM_BRIDGE_PREFIX
#undef  __PWM_BRIDGE_PREFIX
#undef PWM_BRIDGE_BOOT_DELAY
#undef PWM_BRIDGE_SHUTDOWN_DELAY
#undef PWM_BRIDGE_BOOT
#undef PWM_BRIDGE_ON
#undef PWM_BRIDGE_OFF
#undef PWM_BRIDGE_SHUTDOWN
#undef PWM_BRIDGE_SET_PWM
#undef PWM_BRIDGE_PWM_MAX
#undef PWM_BRIDGE_PWM_SOURCE
#endif
