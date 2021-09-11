#ifndef mexo_test_hpp
#define mexo_test_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/ps.hpp"
class pwm  {
public:
	typedef ::mexo::signal_t input_t;
	typedef int16_t duty_t;
	typedef duty_t output_t;
private:
	::mexo::fdc<::mexo::signal_t,duty_t> 	fdc_;
	duty_t duty_;
public:
	struct config_s {
		::mexo::iblock::config_s block;
		::mexo::range_s<duty_t> range;
		float scale;
	};
protected:
	
	void boot_complete(duty_t _duty);
	static void shutdown_begin(void);
	void do_run(duty_t _duty);

	static void boot_begin(void){};
	static bool do_boot(void) { return true; }

	static bool do_shutdown(void) { return true; }
	static void shutdown_complete(void) {  }

	::mexo::iblock::satstate dirrect(const input_t& _deseired, const ::mexo::range_s<output_t>& _range, duty_t& _duty) {		
		return fdc_.dirrect(_deseired, _range, _duty);
	}
	void revert(duty_t _duty, input_t & _actual) {
		fdc_.revert(_duty, _actual);
	}
	bool applay(const config_s& _config) {
		ROBO_LBREAKN(_config.scale > 1.f / 32767); 
		fdc_.scale = _config.scale;
		return true;
	}


};
typedef ::mexo::controller_block_t< ::mexo::ps::pwm<pwm>  > dc;
extern dc::config_s dc_config;

struct current_adc{
	typedef uint32_t native_t;
	static native_t sence[2];
	static void query(void){};
};
typedef ::mexo::sence_block_t< ::mexo::adc_diff<current_adc, ::mexo::signal_t>  > current_sensor_t;

extern dc::config_s dc_config;
extern current_sensor_t::config_s current_sensor_config;



#endif
