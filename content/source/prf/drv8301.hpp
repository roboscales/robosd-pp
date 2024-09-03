#ifndef drv8301_hpp
#define drv8301_hpp
#include "burst/modules/drv8301_reg.h"
#include "core/robosd_system.hpp"

namespace robo {
	namespace prf {
		template < class D > class drv8301_t: protected D {
			public:
				#define ROBO_PRF_DRV8301_CONFIG(h) ROBO_PRF_DRV8301_CONFIG_(h)
				#define ROBO_PRF_DRV8301_CONFIG_(h)\
				{\
					{\
						{\
							h##_GATE_CURRENT\
							, h##_GATE_RESET\
							, h##_PWM_MODE\
							, h##_OCP_MODE\
							, h##_OC_ADJ_SET\
						}\
						, {\
							h##_OCTW_MODE\
							, h##_GAIN\
							, h##_DC_CAL_CH1\
							, h##_DC_CAL_CH2\
							, h##_OC_TOFF\
						}\
					}\
					, {\
							h##_REPEAT_COUNT\
						, h##_REPEAT_DELAY_TICK_COUNT\
					}\
				}
				
				struct  config_s{
					drv8301_config_t  drv;
					struct{ 
						uint16_t count;
						uint32_t delay_ticks_count;
					} repeat;				
				};
				struct present_s{
					drv8301_regs_t actual;
					drv8301_regs_t deseired;
					drv8301_poll_status_t poll_status;

				};
			private:
				config_s & config_;
				present_s & present_;
				
				void phy_exchange_( uint16_t _outcom, uint16_t & _incom){
					D::cs_off();
					D::delay();
					D::put(_outcom);

					while( D::complete() == 0 );
					D::cs_on();
					D::delay();
					_incom = D::get();
				}
			
				void  phy_read_(drv8301_registr_t &  _reg ){
					uint16_t dummy = 0xFFF1;
					uint16_t answer = 0xFFF1;
					uint16_t addr = _reg.address;
					addr = addr<<11;
					uint16_t request = (1<<15) | addr;
					phy_exchange_(request,dummy);
					phy_exchange_(0,answer);
					if( (answer & 0x8000) == 0 && (answer & 0x7800)==addr){
						_reg.data = answer;
					}
				}
				
				void phy_write_( const drv8301_registr_t  _deseired, drv8301_registr_t & _actual){
					int rpt =config_.repeat.count;
					do {

						uint16_t dummy=0xFFF1;
						uint16_t answer=0xFFF1;
						
						uint16_t addr = ((drv8301_registr_p)&_deseired)->address;
						uint16_t request = (1<<15) | (addr<<11);

						phy_exchange_ (_deseired.value, dummy);
						phy_exchange_ (request, dummy);
						phy_exchange_ (0,answer);

						if( answer == _deseired.value ){
							_actual.value = answer;
							return;
						}
						auto delay =  config_.repeat.delay_ticks_count;
						while( delay-- ) D::delay();
					} while(--rpt);
					//D::crash();	
				}


			public:

			void begin(void){
				present_ . poll_status  = DRV8301_POLL_STATUS_NONE;

				uint16_t * d = present_ . deseired.regs;
				for(int i=0; i<DRV8301_REG_COUNT; ++i,++d ){
					*d = (i<<11);
				}
	
				uint16_t * a = present_ . actual.regs;
				for(int i=0; i<DRV8301_REG_COUNT; ++i,++a ){
					*a = (i<<11)  ;
				}

				present_.deseired.control1.GATE_CURRENT = config_.drv.CONTROL1.GATE_CURRENT;
				present_.deseired.control1.GATE_RESET = config_.drv.CONTROL1.GATE_RESET;
				present_.deseired.control1.PWM_MODE = config_.drv.CONTROL1.PWM_MODE;
				present_.deseired.control1.OCP_MODE = config_.drv.CONTROL1.OCP_MODE;
				present_.deseired.control1.OC_ADJ_SET = config_.drv.CONTROL1.OC_ADJ_SET;


				present_.deseired.control2.OCTW_MODE = config_.drv.CONTROL2.OCTW_MODE;
				present_.deseired.control2.GAIN = config_.drv.CONTROL2.GAIN;
				present_.deseired.control2.DC_CAL_CH1 = config_.drv.CONTROL2.DC_CAL_CH1;
				present_.deseired.control2.DC_CAL_CH2 = config_.drv.CONTROL2.DC_CAL_CH2;
				present_.deseired.control2.OC_TOFF = config_.drv.CONTROL2.OC_TOFF;

				phy_read_( present_.actual.status1.reg);
				phy_read_( present_.actual.status2.reg);
				phy_write_( present_.deseired.control1.reg,   present_.actual.control1.reg);
				phy_write_( present_.deseired.control2.reg,   present_.actual.control2.reg);
			}
			void reset(void){
				auto tmp = present_.deseired.control1;
				tmp.GATE_RESET = 1;
				
				D::cs_off();
				D::delay();
				D::put(tmp.value);
				while( D::complete() == 0 );
				D::cs_on();
				D::delay();			
				
			}
			void status_poll(void){
				auto tmp1  = present_.actual.status1;				
				tmp1.FAULT= 1;
				phy_read_ (tmp1.reg);
				
				auto tmp2  = present_.actual.status2;				
				tmp2.unused1=1;
				phy_read_ (tmp2.reg);
				
				{
					robo::system::guard g__;
					present_.actual.status1 = tmp1;
					present_.actual.status2 = tmp2;
				}
				D::on_status_receive();
			}
			/*
			void set_default(drv8301_config_p _config){
				_config->CONTROL1.GATE_CURRENT = DRV8301_GATE_CURRENT_1700mA;
				_config->CONTROL1.GATE_RESET =  DRV8301_GATE_RESET_NORMAL_MODE;
				_config->CONTROL1.OCP_MODE = DRV8301_OCP_MODE_CURRENT_LIMIT;
				_config->CONTROL1.OC_ADJ_SET = DRV8301_OC_ADJ_SET_0060mV;
				_config->CONTROL1.PWM_MODE = DRV8301_PWM_MODE_6_INPUTS;
				
				_config->CONTROL2.DC_CAL_CH1 = DRV8301_DC_CAL_CH1_NORMAL;
				_config->CONTROL2.DC_CAL_CH2 = DRV8301_DC_CAL_CH2_NORMAL;
				_config->CONTROL2.GAIN = DRV8301_GAIN_10VdV;
				_config->CONTROL2.OCTW_MODE = DRV8301_OCTW_MODE_BOTH;
				_config->CONTROL2.OC_TOFF = DRV8301_OC_TOFF_CYCLE_BY_CYCLE;	
			}*/
			drv8301_t(config_s & _config, present_s & _present) : config_(_config), present_(_present) {}
		};
	}
}
#endif
