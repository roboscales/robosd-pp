#ifndef mexo_test_hpp
#define mexo_test_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/ps.hpp"
#include "mexo/enco.hpp"
#include "prf/AS5048A/as5048a.hpp"
#include "net/robosd_flow.hpp"
#include "net/robosd_flow_id.h"


typedef  ::mexo::fixed_point<::mexo::int15> types;

struct  pwm_driver  {	
	static void boot_complete(types::discret_t _duty);
	static void shutdown_begin(void);
	static void do_run(types::discret_t _duty);

	static void boot_begin(void){};
	static bool do_boot(void) { return true; }

	static bool do_shutdown(void) { return true; }
	static void shutdown_complete(void) {  }
};

typedef ::mexo::ps::pwm_block_t <
	types
	, pwm_driver
	, ::mexo::prioritet_subsystem
> dc_power_supply;

struct current_adc_driver{
	static uint32_t sence[2];
	static void query(void){};
};
typedef ::mexo::sence_block_t <
	::mexo::diff_adc<types, current_adc_driver>
	, ::mexo::prioritet_subsystem
> current_sensor;

class as5048_driver{
	protected:
	static void cs_low(void);
	static void cs_hi(void);
	static void put(uint16_t & _commamd);
	static void get(uint16_t & _answer);
};
typedef ::robo::prf::AS5048A<as5048_driver>  AS5048A;

class as5040_driver{
public:
	typedef uint16_t  unative_t;
	typedef int16_t  native_t;
	static uint16_t  native(void);
	static bool  error(void) { return false; };
};


typedef ::mexo::sence_block_t<::mexo::enco::increment<types, AS5048A>, ::mexo::periodic_subsystem > motor_enco;

typedef ::mexo::sence_block_t<::mexo::enco::increment<types, as5040_driver> , ::mexo::periodic_subsystem > motor_quadr_enco;

struct perephery_config_s{
	dc_power_supply::config_s dc_power_supply;
	current_sensor::config_s current_sensor;
	motor_enco::config_s motor_enco;
	motor_quadr_enco::config_s motor_quadr_enco;
};
extern perephery_config_s perephery_config;

void decode_action(const uint8_t * _data, int _sz);
void encode_feetback(uint8_t * _data);


#define can0_PATH RT("can0")

#define echo_PATH RT("echo")
#define echo_KIND backend

#define echo_can0_SUBA 0x1
#define echo_can0_SUBA_ANSW 0x1

#define exchange_PATH RT("exchange")
#define exchange_KIND backend
#define exchange_can0_SUBA 0x2
#define exchange_can0_SUBA_ANSW 0x2

#define serial0_PATH RT("serial0")
#define serial0_KIND frontend
#define serial0_can0_SUBA 0xF
#define serial0_can0_SUBA_ANSW 0xF

#endif
