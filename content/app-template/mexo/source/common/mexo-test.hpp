#ifndef mexo_test_hpp
#define mexo_test_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/ps.hpp"
class pwm  {
public:
	typedef ::mexo::signal_t deseired_t;
	typedef int16_t actual_t;
private:
	::mexo::fdc<::mexo::signal_t,int16_t> 	fdc_;
	int16_t duty_;
public:
	struct config_s {
		::mexo::iblock::config_s block;
		::mexo::fdc<::mexo::signal_t,int16_t>::config_s converter;
	};
protected:
	
	void boot_complete(actual_t _duty);
	static void shutdown_begin(void);
	void do_run(actual_t _duty);

	static void boot_begin(void){};
	static bool do_boot(void) { return true; }

	static bool do_shutdown(void) { return true; }
	static void shutdown_complete(void) {  }

	::mexo::iblock::satstate dirrect(deseired_t _deseired, actual_t& _duty) {		
		duty_ = _duty;
		return fdc_.dirrect(_deseired, _duty);
	}
	void revert(actual_t _duty, deseired_t & _actual) {
		fdc_.revert(_duty, _actual);
	}
	bool applay(const config_s& _config) {
		if (_config.converter.lo < _config.converter.up && _config.converter.scale > 1.f / 32767) {
			fdc_.config = _config.converter;
			return true;
		}
		else {
			return false;
		}
	}
};
typedef ::mexo::controller_block_t< ::mexo::ps::pwm<pwm>  > dc;
typedef ::mexo::controller_block_t< ::mexo::ramp< ::mexo::signal_t > > voltage;
extern dc::config_s dc_config;
#endif
