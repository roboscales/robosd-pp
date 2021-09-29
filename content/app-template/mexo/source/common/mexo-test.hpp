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

typedef ::mexo::ps::pwm_block_t <
	types
	, pwm
	, ::mexo::prioritet_subsystem
> dc_power_supply;

struct current_adc{
	static uint32_t sence[2];
	static void query(void){};
};
typedef ::mexo::sence_block_t <
	::mexo::diff_adc<types, current_adc>
	, ::mexo::prioritet_subsystem
> current_sensor;

extern dc_power_supply::config_s dc_power_supply_config;
extern current_sensor::config_s current_sensor_config;



#endif
