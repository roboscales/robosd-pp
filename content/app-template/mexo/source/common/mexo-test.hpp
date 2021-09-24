#ifndef mexo_test_hpp
#define mexo_test_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/ps.hpp"

typedef  ::mexo::fixed_point<::mexo::int15> types;

class pwm  {
public:
protected:
	
	void boot_complete(types::discret_t _duty);
	static void shutdown_begin(void);
	void do_run(types::discret_t _duty);

	static void boot_begin(void){};
	static bool do_boot(void) { return true; }

	static bool do_shutdown(void) { return true; }
	static void shutdown_complete(void) {  }
};
typedef ::mexo::ps::pwm_b<types, pwm>  dc_power_supply_b;

struct current_adc{
	typedef uint32_t native_t;
	typedef uint32_t acc_t;
	static native_t sence[2];
	static void query(void){};
};
typedef ::mexo::adc_diff_b<current_adc, types>  current_sensor_b;

extern dc_power_supply_b::config_s dc_power_supply_config;
extern current_sensor_b::config_s current_sensor_config;



#endif
