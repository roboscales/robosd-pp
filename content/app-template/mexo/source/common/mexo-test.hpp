#ifndef mexo_test_hpp
#define mexo_test_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/ps.hpp"
class pwm  {
public:
	typedef int16_t duty_t;
protected:
	
	void boot_complete(duty_t _duty);
	static void shutdown_begin(void);
	void do_run(duty_t _duty);

	static void boot_begin(void){};
	static bool do_boot(void) { return true; }

	static bool do_shutdown(void) { return true; }
	static void shutdown_complete(void) {  }
};
typedef ::mexo::ps::pwm_b<pwm>  dc_power_supply_b;

struct current_adc{
	typedef uint32_t native_t;
	typedef uint32_t acc_t;
	static native_t sence[2];
	static void query(void){};
};
typedef ::mexo::adc_diff_b<current_adc, ::mexo::signal_t>  current_sensor_b;

extern dc_power_supply_b::config_s dc_power_supply_config;
extern current_sensor_b::config_s current_sensor_config;



#endif
