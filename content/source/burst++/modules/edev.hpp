#ifndef burst_edev_hpp
#define burst_edev_hpp
#include "core/robosd_common.hpp"
#include "im/edev/edev.hpp"
#include "core/robosd_ini.hpp"
#include <math.h>
#include "im/models/power.hpp"

namespace burst {
	namespace edev {
		template< typename T = void> class ROBO_EXPORT agent_t : public robo::edev::agent {
		protected:
#if ROBO_APP_BURST_VARTREE_ENABLED == 1
			virtual void do_regvar() = 0;
#endif
		public:
			class ROBO_EXPORT block : public robo::edev::agent::block {
			public:
				typedef robo::list::unsorted<block> list;
				typedef typename list::ref ref;
			protected:
				#if ROBO_APP_BURST_VARTREE_ENABLED == 1
				virtual void do_regvar(void) = 0;
				#endif
			public:
				block(agent_t& _agent, robo::cstr _name) :robo::edev::agent::block(_agent, _name) {}
#if ROBO_APP_BURST_VARTREE_ENABLED == 1
				void regvar(void) {
					using namespace burst::var;
					push(name);
					do_regvar();
					pop();
				}
#endif				
			};

#if ROBO_APP_BURST_VARTREE_ENABLED == 1
			void regvar(void) {
				using namespace burst::var;
				push(RT("im"));
				do_regvar();
				for (robo::edev::agent::block::ref* p = blocks.first(); p; p = p->next()) {
					block* bp = dynamic_cast<block*>(&(p->owner()));
					if (bp) bp->regvar();
				}
				pop();
			}
#endif				

		};
		template< typename T> class hall_t :public agent_t<T>::block {
		public:
			typedef union {
				struct {
					unsigned int A : 1;
					unsigned int B : 2;
					unsigned int C : 3;
				};
				unsigned int code;
			} hall_s;
			const T& position_el_rad;
			hall_s& abc;
			hall_t(agent_t<T>& _agent, robo::cstr _name, const T& _position_el_rad, hall_s& _abc)
				: agent_t<T>::block(_agent, _name)
				, position_el_rad(_position_el_rad)
				, abc(_abc)
			{}


			static const inline hall_s sectors_value[6] = {
				{ 1, 0, 0 }
				, { 1, 1, 0 }
				, { 0, 1, 0 }
				, { 0, 1, 1 }
				, { 0, 0, 1 }
				, { 1, 0, 1 }
			};

			float offset_el_grad = 0.f;
			float noise_el_grad = 0.f;
			float noise_el_mag_grad = 0.f;
			T position_el_grad = 0.f;

			uint8_t sector = 0;

			virtual bool do_load(robo::cstr _specific_sect, robo::cstr _common_sect) {
				ROBO_LBREAKN(robo::ini::load(_specific_sect, _common_sect, RT("noise_el_mag_grad"), noise_el_mag_grad));
				ROBO_LBREAKN(robo::ini::load(_specific_sect, _common_sect, RT("offset_el_grad"), offset_el_grad));
				return true;
			}

			virtual void do_reconfig(void) {

			}

			virtual void do_run(void) {
				noise_el_grad = ::robo::system::rand(-noise_el_mag_grad, noise_el_mag_grad);
				position_el_grad = (float)position_el_rad * robo::rad2deg<float> +noise_el_grad + 30;
				while (position_el_grad < 0.f) position_el_grad += 360.f;
				while (position_el_grad >= 360.f) position_el_grad -= 360.f;
				sector = (uint8_t)(position_el_grad / 60.f);
				abc.code = sectors_value[sector].code;
			}
			#if ROBO_APP_BURST_VARTREE_ENABLED == 1
			virtual void do_regvar(void) {
				using namespace burst::var;
				reg(types::real, offset_el_grad, RT("offset_el_grad"));
				reg(types::real, noise_el_mag_grad, RT("noise_el_mag_grad"));
				reg(types::const_real, position_el_grad, RT("position_el_grad"));
				reg(types::const_uint8, sector, RT("sector"));
			}
			#endif
		};
		template< typename T> class ssi_resolver_t :public agent_t<T>::block {
		public:
			T& position; // rad положение датчика
			uint8_t resolution;// = 15;// pp Разрешение датчика положения , 
			uint8_t poles;// = 32;
			float gain;// = (float)((1L << 14) * poles) / ::robo::pi<float>;// pp /rad коэффициент между попугаями драйвера и углом поворота роторашарнира
			float noizeMag = 0;//rad амплитуда шума на датчике а рад
			uint8_t noizeBit = 0;//bit  Шум датчика относительно драйвера
			uint32_t packet = 0;
			float offset_grad = 0.;
			float offset_rad = 0.;
			ssi_resolver_t(agent_t<T>& _agent, robo::cstr _name, T& _position, uint8_t _resolution, uint8_t _poles)
				: agent_t<T>::block(_agent, _name)
				, position(_position)
				, resolution(_resolution)
				, poles(_poles)
				//, gear_ratio(_gear_ratio)
			{
				gain = (float)((1L << (resolution - 1))) / ::robo::pi<float>;// pp /rad коэффициент между попугаями драйвера и углом поворота роторашарнира
			}

			virtual void do_run(void) {
				//140-150ns
				//1. Фактическое положение датчика в радианах						
				float noize = ::robo::system::rand(-noizeMag, noizeMag);
				//noizeMag* (float)(rand() % 1000 - 500) / 500.f;
				double tmp = gain * fmod(position * poles + offset_rad + noize, 2.f * ::robo::pi<float>);

				if (tmp > std::numeric_limits<double>::epsilon()) {
					tmp += 0.5;
					packet = (uint32_t)(tmp);
				}
				else if (tmp < -std::numeric_limits<double>::epsilon()) {
					tmp -= 0.5;
					packet = ((1 << resolution) - 1) - (uint32_t)(-tmp);
				}
			}
			virtual bool do_load(robo::cstr _specific_sect, robo::cstr _common_sect) {
				//140-150ns
				ROBO_LBREAKN(robo::ini::load(_specific_sect, _common_sect, RT("noizeBit"), noizeBit));
				ROBO_LBREAKN(robo::ini::load(_specific_sect, _common_sect, RT("offset_grad"), offset_grad));
				offset_rad = offset_grad * robo::deg2rad<float>;
				if (noizeBit == 0) {
					noizeMag = 0.0;
				}
				else {
					noizeMag = (2.0f * ::robo::pi<float>) / (1 << resolution) * (1 << (noizeBit - 1));
				}
				return true;
			}
			#if ROBO_APP_BURST_VARTREE_ENABLED == 1
			virtual void do_regvar(void) {
				using namespace burst::var;
				reg(types::const_ext, position, RT("position"));
			}
			#endif
			virtual void do_reconfig(void) {
			}
		};
		template<typename T> class sicos_resolver_t :public agent_t<T>::block {
		public:
			T& position; // rad положение  датчика  (с учетом смещения и знака)
			//генератор синуса
			struct {
				float value = 0.f;
				float phase = 0.f;
				robo::time_us_t offset_us = 0;
			} outcom;
			//жаннве СКВТ
			struct {
				struct {
					float sn = 0;
					float cs = 0;
				} raw;
				struct {
					float sn = 0;
					float cs = 0;
				} v;
				struct {
					uint32_t sn = 0;
					uint32_t cs = 0;
				} adc;
				float gain;
				float ampV;//rad амплитуда sincos
				float zeroV;//rad амплитуда sincos
				uint8_t resolution;//bit  разрядность
				float noizeMagV;//rad амплитуда шума на датчике
				float revV;//
				uint8_t noizeBit;//bit  Шум датчика относительно драйвера
				float max;
			} incom = {};
			sicos_resolver_t(agent_t<T>& _agent, robo::cstr _name, T& _position)
				: agent_t<T>::block(_agent, _name)
				, position(_position)
			{
			}
			float mess_v(float _value) {
				float noizeV = incom.noizeMagV * (float)(rand() % 1000 - 500) / 500.f;
				return (_value * incom.ampV + noizeV + incom.zeroV);
			}
			uint32_t mess_n(float _value) {
				float tmp = _value * incom.gain;
				if (tmp > 0.)  tmp += 0.5; else if (tmp < 0.)	tmp -= 0.5; // округление данных датчика
				if (tmp < 0.) tmp = 0.;
				if (tmp > incom.max) tmp = incom.max;
				return (uint32_t)tmp;
			}

			virtual bool do_load(robo::cstr _sect, robo::cstr _default) {
				ROBO_LBREAKN(robo::ini::load(_sect, _default, RT("ampV"), incom.ampV));
				ROBO_LBREAKN(robo::ini::load(_sect, _default, RT("zeroV"), incom.zeroV));
				ROBO_LBREAKN(robo::ini::load(_sect, _default, RT("revV"), incom.revV));
				ROBO_LBREAKN(robo::ini::load(_sect, _default, RT("noizeBit"), incom.noizeBit));
				ROBO_LBREAKN(robo::ini::load(_sect, _default, RT("resolution"), incom.resolution));
				//????? todo
				//outcom.offset_us = (robo::time_us_t)(1000000. / RESOLVER_SINCOS_DRIVER_FREQ / RESOLVER_SINCOS_DRIVER_TABLE_SIZE * RESOLVER_SINCOS_DRIVER_MESS_OFFSET);
				
				//actuatorPositionNoizeMag = (2.0f * ::robo::pi<float> ) / (1 << (actuatorDriverPositionResolutionBit - actuatorDriverNoizePositionBit));

				if (incom.noizeBit == 0) {
					incom.noizeMagV = 0.0;
				}
				else {
					incom.noizeMagV = incom.ampV / (1 << incom.resolution) * (1 << (incom.noizeBit - 1));
				}

				incom.max = (float)(1 << incom.resolution);
				incom.gain = incom.max / incom.revV;

				return true;
			}
			#if ROBO_APP_BURST_VARTREE_ENABLED == 1
			virtual void do_regvar(void) {
				using namespace burst::var;
				reg(types::const_real, outcom.value, RT("o.value"));
				reg(types::const_real, outcom.phase, RT("o.phase"));
				reg(types::uint32, outcom.offset_us, RT("o.offset_us"));
				reg(types::const_real, incom.raw.sn, RT("i.raw.sn"));
				reg(types::const_real, incom.raw.cs, RT("i.raw.cs"));
				reg(types::const_uint32, incom.adc.sn, RT("i.adc.sn"));
				reg(types::const_uint32, incom.adc.cs, RT("i.adc.cs"));
				reg(types::const_real, incom.v.sn, RT("i.v.sn"));
				reg(types::const_real, incom.v.cs, RT("i.v.cs"));
			}
			#endif	

			virtual void do_run(void) {
				//150-170ns
				//1. Фактическое положение датчика в радианах						
				
				//????? todo
				//	outcom.phase = robo::pi<float>*(robo::system::env::realtime_us()) * RESOLVER_SINCOS_DRIVER_FREQ * 2 / 1000000.f;
				//while (outcom.phase > 2. * robo::pi<float>) outcom.phase -= 2. * robo::pi<float>;
				outcom.phase = fmod(outcom.phase, 2.0f * ::robo::pi<float>);
				outcom.value = sin(outcom.phase);
				incom.raw.sn = (float)::sin(position) * outcom.value;
				incom.raw.cs = (float)::cos(position) * outcom.value;
				incom.v.sn = mess_v(incom.raw.sn);
				incom.v.cs = mess_v(incom.raw.cs);
				incom.adc.sn = mess_n(incom.v.sn);
				incom.adc.cs = mess_n(incom.v.cs);
			}
			virtual void do_reconfig(void) {
			}

		};

		template<typename T> class current_sensor_t :public agent_t<T>::block {
		public:
			using raw_s = typename robo::edev::power::ph3_t<T>::sabc;
			const raw_s& raw;
			struct adc_s {
				uint32_t A;
				uint32_t B;
				uint32_t C;
			} adc = {};
			int driverNoizeBit; //Шум датчика тока относительно драйвера
			int driverMax; //максимальный ток относительно АЦП
			int driverZero; // ноль ток относительно АЦП
			float noizeMag;
			float senceMax;
			float gain;
			uint8_t resolution;
			current_sensor_t(agent_t<T>& _agent, robo::cstr _name, const raw_s& _raw, uint8_t _resolution)
				: agent_t<T>::block(_agent, _name)
				, raw(_raw)
				, resolution(_resolution)
			{
			}

			uint32_t sence_current(T _value) {
				T noize = ::robo::system::rand(-noizeMag, noizeMag);
				T tmp = (_value + noize) * gain;
				if (tmp > 0)  tmp += 0.5; else if (tmp < 0)	tmp -= 0.5; // округление данных датчика

				int32_t ret = driverZero + (int32_t)tmp;
				if (ret < 0) ret = 0;
				if (ret > driverMax) ret = driverMax;
				return (uint32_t)ret;
			}

			virtual void do_run(void) {
				//140-150ns????
				adc.A = sence_current(raw.A);
				adc.B = sence_current(raw.B);
				adc.C = sence_current(raw.C);
			}
			virtual bool do_load(robo::cstr _specific_sect, robo::cstr _common_sect) {
				ROBO_LBREAKN(robo::ini::load(_specific_sect, _common_sect, RT("senceMax"), senceMax));
				ROBO_LBREAKN(robo::ini::load(_specific_sect, _common_sect, RT("driverNoizeBit"), driverNoizeBit));
				driverMax = (1 << resolution) - 1;
				driverZero = driverMax >> 1;
				gain = (float)driverMax / (2.0f * senceMax);
				noizeMag = (2.0f * senceMax) / (1 << (resolution - driverNoizeBit));
				return true;
			}
			#if ROBO_APP_BURST_VARTREE_ENABLED == 1
			virtual void do_regvar(void) {
			}
			#endif
			virtual void do_reconfig(void) {
			}
		};
	
		template<typename I, typename O> class adc_t :public agent_t<O>::block {
			O beta_ = 0.1f;
			O scale_ = 1.f;
			O volt2pp_ = 1.f;
			O offset_ = 0.f;
			uint8_t adc_bit_ = 12;
			O noize_mag_ = 0.f;
			uint8_t noise_bits_ = 1;
			O noize_power_ = 1.f;
			O max_pp_ = 1;
			const I& in_;
		public:
			O adc_ref = 3.3f;
			O voltage = 0.f;
			uint32_t pp = 0;

		protected:
			robo::string type;
			
			virtual bool do_load(robo::cstr _specific_sect, robo::cstr _common_sect) {
				robo::string stype(RT("%s.%s"), owner.ctype(), type.c_str());
				ROBO_LBREAKN(robo::ini::load(_specific_sect, stype, RT("beta"), beta_));
				ROBO_LBREAKN(robo::ini::load(_specific_sect, stype, RT("scale"), scale_));
				ROBO_LBREAKN(robo::ini::load(_specific_sect, stype, RT("offset_pp"), offset_));
				ROBO_LBREAKN(robo::ini::load(_specific_sect, stype, RT("adc_bits"), adc_bit_));
				ROBO_LBREAKN(robo::ini::load(_specific_sect, stype, RT("adc_ref"), adc_ref));
				ROBO_LBREAKN(robo::ini::load(_specific_sect, stype, RT("noise_bits"), noise_bits_));
				ROBO_LBREAKN(robo::ini::load(_specific_sect, stype, RT("noize_power"), noize_power_));
				return true;
			}
			virtual void do_run(void) {
				float noize = ::robo::system::rand(0.f, 1.f);
				noize = ::powf(noize, noize_power_);
				noize = (noize * 2.f - 1.f) * noize_mag_;
				float tmp = in_ * scale_;
				voltage = voltage * beta_ + tmp * (1 - beta_) + noize;
				tmp = voltage * volt2pp_ + offset_ + 0.5f;
				if (tmp > max_pp_) {
					tmp = max_pp_;
				}
				else if (tmp < 0) {
					tmp = 0.f;
				}
				pp = (uint32_t)tmp;
			}
			virtual void do_regvar(void) {
				#if ROBO_APP_BURST_VARTREE_ENABLED == 1
				using namespace burst::var;
				reg(types::const_real, voltage, RT("v"));
				reg(types::uint32, pp, RT("pp"));
				#endif
			}
			virtual void do_reconfig(void) {
				max_pp_ = (::powf(2, adc_bit_) - 1);
				volt2pp_ = max_pp_ / adc_ref;
				noize_mag_ = (::powf(2, noise_bits_) - 1) / volt2pp_;
			}
		public:
			adc_t(agent_t<O>& _agent, robo::cstr _name, robo::cstr _type, I& _in)
				: ::swana::edev::agent::block(_agent, _name)
				, in_(_in)
				, type(_type)
			{
			}
		};
	}
}
			
#endif