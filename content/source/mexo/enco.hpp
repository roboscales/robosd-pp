#ifndef mexo_sence_hpp
#define mexo_sence_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	namespace enco {
		template <
			typename q
			, typename D
			, uint8_t round_resolution
			, uint8_t raw_resolution
			, uint8_t actual_resolution
			, typename native_t
			, typename unative_t
			, typename doutput_t
			, typename output_t
		> class abs_machine
			: public handler, public D {
		public:
			static  inline const  int shift = (round_resolution - raw_resolution);
			static  inline const int value_shift = (raw_resolution - actual_resolution);
			struct config_s {
				handler::config_s	sb;
				uint8_t init_count_shift;
				unative_t native_offset;
				output_t position_offset;
				bool inverce;
			};
			unative_t native_offset_prev =(unative_t)0;
			output_t position_offset_prev = (output_t)0;
			uint32_t start_pause_tick;

			struct present_s {
				handler::present_s	sb;
				struct {
					unative_t raw;
					unative_t ceiled;
					native_t delta;
				} native;
				struct{
					unsigned fault;
					unsigned total;
				} counter;
				doutput_t delta;
				output_t acc;
				output_t position;
				doutput_t delta_acc;
			};
		protected:
			virtual void do_handler_adjust(void) {
			}

			void execute(void) {				
				present_s& prsnt = present<abs_machine>();
				const config_s& conf = config<abs_machine>();
				if (conf.native_offset != native_offset_prev || conf.position_offset != position_offset_prev) {
					do_handler_reconfig();
					return;
				}
				prsnt.counter.total++;
				prsnt.native.raw = D::encode();
				D::query();
				if (start_pause_tick == 0) {
					if ( !D::error() ) {
						unative_t tmp = prsnt.native.raw << shift;
						native_t  tmp_delta = (native_t)(tmp - prsnt.native.ceiled);
						prsnt.native.ceiled = tmp;
						//todo проверить на всехли компиляторах shift будет арифметический
						prsnt.native.delta = (native_t)(tmp_delta >> shift);// (((native_t)(tmp_delta)) >> shift);
						output_t dtmp;
						if (conf.inverce) {
							 dtmp = -q::round_l(prsnt.native.delta , value_shift);
						}
						else {
							dtmp = q::round_l(prsnt.native.delta, value_shift);
						}
						if(dtmp > std::numeric_limits<doutput_t>::max()){
							prsnt.delta = prsnt.delta_acc = std::numeric_limits<doutput_t>::max();
						} else if(dtmp < -std::numeric_limits<doutput_t>::max()) {
							prsnt.delta = prsnt.delta_acc = -std::numeric_limits<doutput_t>::max();
						} else {
							prsnt.delta = dtmp;
							output_t adtmp =  (output_t)prsnt.delta_acc + prsnt.delta;
							if(dtmp > std::numeric_limits<doutput_t>::max()){
								prsnt.delta_acc = std::numeric_limits<doutput_t>::max();
							} else if(dtmp < -std::numeric_limits<doutput_t>::max()) {
								prsnt.delta_acc = -std::numeric_limits<doutput_t>::max();
							}	else {						
								prsnt.delta_acc += prsnt.delta;
							}
						}
					}
					else {
						prsnt.counter.fault++;
						//to do так делать нельзя, та как накапливается ошибка!
						prsnt.native.delta = prsnt.delta_acc = 0;
						/*present.native.raw += present.native.delta;
						present.native.ceiled += (present.native.delta << shift);
						present.delta_acc += present.delta;*/
					}
					if (conf.inverce) {
						prsnt.acc -= prsnt.native.delta;
					}
					else {
						prsnt.acc += prsnt.native.delta;
					}
					//todo round_l не катит
					prsnt.position = q::round_l(prsnt.acc, value_shift);
					prsnt.position += conf.position_offset;
				}
				else {
					if ( !D::error() )  {
						prsnt.native.ceiled = prsnt.native.raw << shift;
						unative_t tmp = prsnt.native.ceiled + conf.native_offset;
						if (conf.inverce) {
							tmp = std::numeric_limits<unative_t>::max() - tmp;
						}
						prsnt.acc = (output_t)(native_t)tmp;
						prsnt.acc = q::round_l(prsnt.acc, shift );
						prsnt.position = q::round_l(prsnt.acc, value_shift );
						prsnt.position += conf.position_offset;
						start_pause_tick--;
					}
					else {
						if(D::error())
							prsnt.counter.fault++;
					}

				}
			}
			#if ROBO_APP_MEXO_VAR_ENABLED == 1
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				handler::do_handler_create_vars(_vars, _master_key);
				present_s& prsnt = present<abs_machine>();
				if (var::machine::actual_mode() >= var::machine::mode::full) {
					if (round_resolution == 32) {
						var::record::create(::mexo::var::uint32, prsnt.native.raw, RT("native"), _master_key, _vars);
					}
					else if (round_resolution == 16){
						var::record::create(::mexo::var::uint16, prsnt.native.raw, RT("native"), _master_key, _vars);
					}
					var::record::create(::mexo::var::uint32, prsnt.counter.fault, RT("cnt.fault"), _master_key, _vars);
					var::record::create(::mexo::var::uint32, prsnt.counter.total, RT("cnt.tot"), _master_key, _vars);
					var::record::create(q::var::signal, prsnt.delta, RT("delta"), _master_key, _vars);
					var::record::create(q::var::signal, prsnt.delta_acc, RT("delta_acc"), _master_key, _vars);
					var::record::create(q::var::long_signal, prsnt.position, RT("po"), _master_key, _vars);
				}

				if (var::machine::actual_mode() >= var::machine::mode::tuning) {
					const config_s& cfg = config<abs_machine>();
					if (round_resolution == 32) {
						var::record::create(::mexo::var::uint32, cfg.native_offset, RT("native_offset"), _master_key, _vars);
					} else if (round_resolution == 16) {
						var::record::create(::mexo::var::uint16, cfg.native_offset, RT("native_offset"), _master_key, _vars);
					}
					//todo govnocod
					switch( sizeof(cfg.position_offset)){
						case 2:
							var::record::create(::mexo::var::int16, cfg.position_offset, RT("position_offset"), _master_key, _vars);
							break;
						case 4:
							var::record::create(::mexo::var::int32, cfg.position_offset, RT("position_offset"), _master_key, _vars);
							break;
						case 8:
							var::record::create(::mexo::var::int64, cfg.position_offset, RT("position_offset"), _master_key, _vars);
							break;
					}
					var::record::create(::mexo::var::uint8, cfg.inverce, RT("inverce"), _master_key, _vars);
				}
			}
			#endif

			virtual bool do_handler_reconfig(void) {
				present_s& prsnt = present<abs_machine>();
				const config_s& conf = config<abs_machine>();
				prsnt.native = {};
				prsnt.counter = {};
				start_pause_tick = 1 << conf.init_count_shift;
				position_offset_prev = conf.position_offset;
				native_offset_prev = conf.native_offset;
				D::query();
				return true;
			}
		public:
			abs_machine(const config_s& _config, present_s& _present)
				: handler(_config.sb, _present.sb) {}
			const doutput_t& delta_ref(void) { return  present<abs_machine>().delta; }
			doutput_t& delta_acc_ref(void) { return  present<abs_machine>().delta_acc; }
			const output_t& position_ref(void) { return  present<abs_machine>().position; }
		};

		template <typename q, typename D, uint8_t raw_resolution, uint8_t actual_resolution > class abs32_t
			: public abs_machine <
					q
					, D
					, 32
					, raw_resolution
					, actual_resolution
					, int32_t
					, uint32_t
					, typename q::signal_t
					, typename q::long_signal_t
				>
			{
			typedef abs_machine <
				q
				, D
				, 32
				, raw_resolution
				, actual_resolution
				, int32_t
				, uint32_t
				, typename q::signal_t
				, typename q::long_signal_t
			> A;
			protected:

		public:
			abs32_t(const typename A::config_s& _config, typename A::present_s& _present)
				: A(_config, _present) {}
		};

		template <typename q, typename D, uint8_t raw_resolution, uint8_t actual_resolution> class abs16_t
			: public
			abs_machine <
			q
			, D
			, 16
			, raw_resolution
			, actual_resolution
			, int16_t
			, uint16_t
			, typename q::signal_t
			, typename q::long_signal_t
			>
		{
			typedef abs_machine <
				q
				, D
				, 16
				, raw_resolution
				, actual_resolution
				, int16_t
				, uint16_t
				, typename q::signal_t
				, typename q::long_signal_t
			> A;
		protected:

		public:
			abs16_t(const typename A::config_s& _config, typename A::present_s& _present)
				: A(_config, _present) {}
		};

		template <typename q, typename D, uint8_t raw_resolution, uint8_t actual_resolution > class abs64_t
			: public
			abs_machine <
			q
			, D
			, 32
			, raw_resolution
			, actual_resolution
			, int32_t
			, uint32_t
			, int32_t
			, int64_t
			>
		{
			typedef abs_machine <
				q
				, D
				, 32
				, raw_resolution
				, actual_resolution
				, int32_t
				, uint32_t
				, int32_t
				, int64_t
			> A;
		protected:

		public:
			abs64_t(const typename A::config_s& _config, typename A::present_s& _present)
				: A(_config, _present) {}
		};

		template<typename q, typename U> class rotator_t
			: public function_handler< U, cs_t<q> > {
			typedef function_handler< U, cs_t<q> > A;
		public:
			typedef typename q::signal_t signal_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef  U unative_t;
			struct config_s {
				typename A::config_s fb;
				unative_t offset;
				bool inverce;
				uint16_t pole_count;
			};
			struct present_s {
				typename A::present_s fb;
				bool active;
				signal_t rotor_angle;
			};
		protected:

			#if ROBO_APP_MEXO_VAR_ENABLED == 1
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				A::do_handler_create_vars(_vars, _master_key);
				present_s& present = A::template present<rotator_t>();
				const config_s& config = A::template config<rotator_t>();
				if (var::machine::actual_mode() >= var::machine::mode::full) {
					var::record::create(var::const_uint8, present.active, RT("active"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.fb.output.si, RT("ab.sin"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.fb.output.co, RT("ab.cos"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.fb.output.angle, RT("dq.angle"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.rotor_angle, RT("rotor_angle"), _master_key, _vars);
				}
				if (var::machine::actual_mode() >= var::machine::mode::tuning) {
					var::record::create(::mexo::var::uint16, config.pole_count, RT("pole_count"), _master_key, _vars);
					var::record::create(::mexo::var::uint8, config.inverce, RT("inverce"), _master_key, _vars);
					if (sizeof(unative_t) == sizeof(signal_t)) {
						var::record::create(q::var::usignal, config.offset, RT("offset"), _master_key, _vars);
					}
					else {
						var::record::create(q::var::ulong_signal, config.offset, RT("offset"), _master_key, _vars);
					}
				}
			};
			#endif


			void execute(void) {
				present_s& present = handler::present<rotator_t>();
				const config_s& config = handler::config<rotator_t>();
				unative_t tmp;
				if (config.inverce) {
					tmp = std::numeric_limits<unative_t>::max() - A::input;
				}
				else {
					tmp = A::input;
				}
				tmp *= config.pole_count;
				tmp -= config.offset;
				if (sizeof(unative_t) == sizeof(signal_t)) {
						present.rotor_angle = (signal_t)tmp;
				}
				else {
					present.rotor_angle = q::scale_l((long_signal_t)tmp);
				}
				if (present.active) {
					present.fb.output.rotate( present.rotor_angle);
				}
				
			}
			bool do_handler_reconfig(void) {
				handler::present<rotator_t>().fb.output.rotate((signal_t)0);
				on();
				return true;
			}
			virtual void do_handler_adjust(void) {
				handler::present<rotator_t>().fb.output.rotate(q::scale_l(A::input));
			}
		public:
			rotator_t(const config_s& _config
					  , present_s& _present
					  , unative_t& _input
			)
				: A(_config.fb, _present.fb, _input) {
			}
			void on() { handler::present<rotator_t>().active = true; };
			void off() { handler::present<rotator_t>().active = false; };
			void angle_set(signal_t _angle) {
				present_s& present = handler::present<rotator_t>();
				if (present.active == false) {
					present.fb.output.rotate(_angle);
				}
			}

		};

		template <
			typename q
			, typename position_t
			, typename speed_t
			, typename D
		> class hall_t
			: public handler, public D {
		public:
			using angle_t = typename q::signal_t;
			struct config_s {
				handler::config_s	sb;
				struct {
					struct {
						angle_t native;
						angle_t dynamic;
					} angle;
					position_t position;
				} offset;
				bool inv;
			};
			angle_t angle_offset_prev = (angle_t)0;
			position_t position_offset_prev = (position_t)0;

			static constexpr int8_t sectors[8] = {
				-1 //{ 0, 0, 0 }
				, 4//{ 1, 0, 0 }
				, 0//{ 0, 1, 0 }
				, 5 //{ 1, 1, 0 }
				, 2 //{ 0, 0, 1 }
				, 3 //{ 1, 0, 1 }
				, 1 //{ 0, 1, 1 }
				, -1 //{1, 1, 1}
			};

			angle_t  angles[6] = {
				angle_t(0)
				, angle_t(q::pi/3.)
				, angle_t(2*q::pi /3.)
				, angle_t(q::pi)
				, angle_t(-2 * q::pi / 3.)
				, angle_t(- q::pi / 3.)
			};
			struct present_s {
				handler::present_s	sb;
				struct {
					unsigned fault;
					unsigned total;
				} counter;
				uint8_t pins[3];
				int8_t sector;
				angle_t raw;
				angle_t angle;
				int8_t delta;
				position_t position;
				speed_t delta_acc;
			};
		private:
			int8_t true_diff_ = 0;
			int8_t sector_prev_ = -1;
			int index_ = -1;			
			static constexpr int8_t diffs_[6] = {
				0, 1, 2, 3, -2, -1
			};
			position_t offset_position_prev_;
		protected:
			virtual void do_handler_adjust(void) {}

			void execute(void) {
				present_s& prsnt = present<hall_t>();
				const config_s& conf = config<hall_t>();
				if (conf.offset.position != offset_position_prev_) {
					do_handler_reconfig();
					return;
				}
				prsnt.counter.total++;
				D::read(prsnt.pins);
				index_ = prsnt.pins[0] + (prsnt.pins[1] <<1) + (prsnt.pins[2] <<2);
				if (index_ >= 0) {
					prsnt.sector = sectors[index_];
					if(prsnt.sector>=0){
						prsnt.delta = (int8_t)prsnt.sector - (int8_t)sector_prev_;
						sector_prev_ = prsnt.sector;
						if (prsnt.delta >= 0) {
							prsnt.delta = diffs_[prsnt.delta];
						}
						else {
							prsnt.delta = diffs_[6 + prsnt.delta];
						}
						if (prsnt.delta == 3) {
							if (true_diff_ < 0) {
								prsnt.delta = -3;
							}
						} else {
							true_diff_ = prsnt.delta;
						}
						if (conf.inv) {
							prsnt.delta = -prsnt.delta;
							prsnt.raw = -angles[prsnt.sector];
						}
						else {
							prsnt.raw = angles[prsnt.sector];
						}
						prsnt.delta_acc += prsnt.delta;
						prsnt.angle = prsnt.raw + conf.offset.angle.native;
						if (D::dir() > 0) {
							prsnt.angle += conf.offset.angle.dynamic;
						}
						else if ( D::dir()  < 0) {
							prsnt.angle += conf.offset.angle.dynamic;
						}
						prsnt.position += prsnt.delta;
						return;
					}
				}
				prsnt.counter.fault++;
				//to do так делать нельзя, та как накапливается ошибка!
				prsnt.delta = 0;
				prsnt.delta_acc = 0;
			}
			#if ROBO_APP_MEXO_VAR_ENABLED == 1
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				handler::do_handler_create_vars(_vars, _master_key);
				present_s& prsnt = present<hall_t>();
				::mexo::var::types pos_tp;
				switch (sizeof(position_t)) {
				case 1:
				pos_tp = ::mexo::var::int8;
				break;
				case 2:
				pos_tp = ::mexo::var::int16;
				break;
				case 4:
				pos_tp = ::mexo::var::int32;
				break;
				case 8:
				pos_tp = ::mexo::var::int64;
				break;
				default:
				pos_tp = ::mexo::var::int8;
				}

				if (var::machine::actual_mode() >= var::machine::mode::full) {
					//todo govnocod
					var::record::create(q::var::signal, prsnt.raw, RT("raw"), _master_key, _vars);
					var::record::create(q::var::signal, prsnt.angle, RT("angle"), _master_key, _vars);
					var::record::create(::mexo::var::uint32, prsnt.counter.fault, RT("cnt.fault"), _master_key, _vars);
					var::record::create(::mexo::var::uint32, prsnt.counter.total, RT("cnt.tot"), _master_key, _vars);
					var::record::create(q::var::signal, prsnt.delta, RT("delta"), _master_key, _vars);
					var::record::create(q::var::signal, prsnt.delta_acc, RT("delta_acc"), _master_key, _vars);
					var::record::create(pos_tp, prsnt.position, RT("po"), _master_key, _vars);
				}

				if (var::machine::actual_mode() >= var::machine::mode::tuning) {
					const config_s& cfg = config<hall_t>();
					var::record::create(q::var::signal, cfg.offset.angle.native , RT("native_offset"), _master_key, _vars);
					var::record::create(q::var::signal, cfg.offset.angle.dynamic , RT("dynam_offset"), _master_key, _vars);
					var::record::create(pos_tp, cfg.offset.position, RT("position_offset"), _master_key, _vars);
					var::record::create(::mexo::var::uint8, cfg.inv, RT("inverce"), _master_key, _vars);
				}
			}
			#endif

			virtual bool do_handler_reconfig(void) {
				present_s& prsnt = present<hall_t>();
				const config_s& conf = config<hall_t>();
				prsnt.counter = {};
				prsnt.position = conf.offset.position;
				prsnt.delta_acc = speed_t(0);

				return true;
			}
		public:
			hall_t(const config_s& _config, present_s& _present)
				: handler(_config.sb, _present.sb) {}
			const angle_t& delta_ref(void) { return  present<hall_t>().native.delta; }
			speed_t& delta_acc_ref(void) { return  present<hall_t>().delta_acc; }
			const position_t& position_ref(void) { return  present<hall_t>().position; }
		};

	}
}
#endif