#include "mexo/mexo.hpp"
#ifndef mexo_ps_hpp
#define mexo_ps_hpp
namespace mexo {
	/*class regulator {
	public:
		enum class saturate { none, low, hi };
		block::output_t< saturate > status;
	};*/
	namespace ps {
		struct range_s {
			signal_t lo;
			signal_t hi;
		};
		class  power  {
		public:
			enum class command { on = 1, off = 0 };
			enum class status { configure = 0, off = 1, boot = 2, on = 3, shutdown = 4 };
		private:
		protected:
			signal_t desired = (signal_t)0;
			parametr_t rampGain = (signal_t)0;
			command command_ = command::off;
			status status_ = status::off;
		public:
			block::input_t<signal_t> required;
			void ramp(void) {
				signal_t r = required.value();
/*				if (r >= range.value().hi) {
					r = range.value().hi; 
				}
				else if (r <= range.value().lo) {
					r = range.value().lo;
				}*/
				signal_t delta = r - desired;
				if (delta > 0) {
					if (delta < rampGain) {
						desired = r;
					}
					else {
						desired += rampGain;
					}
				}
				else {
					if (delta < 0) {
						if ((-delta) < rampGain) {
							desired = r;
						}
						else {
							desired -= rampGain;
						}
					}
				}
			}
			void enable(void) { if (status_ == status::configure) status_ = status::off; }
			void on(void) { command_ = command::on; }
			void set(command _command) { command_ = _command; }
			void off(void) { command_ = command::off; }
			bool active(void) { return status_ == status::on; }
			power(void){
			}

		};

		struct config_s {
			block::config_s  block;
			parametr_t rampGain;
		};

	
		template < typename D, typename R > class machine : public D, public power {
		private:
			const R& required_;
		protected:
			machine(const R& _required) :required_(_required) {}
			void execute () {
				switch (power::status_) {
				case status::off:
					if (power::command_ == command::on) {
						D::boot_begin();
						power::status_ = status::boot;
					}
					else {
						break;
					}

				case status::boot:
					if (D::do_boot()) {
						D::boot_complete(required_);
						power::status_ = status::on;
					}
					else {
						break;
					}
				case status::on:
					if (power::command_ == command::on) {
						D::do_run(required_);
						break;
					}
					else {
						power::status_ = status::shutdown;
						D::shutdown_begin();
					}
				case status::shutdown:
					if (D::do_shutdown()) {
						D::shutdown_complete();
						power::status_ = status::off;
					}
					else {
						break;
					}
				case status::configure:
					break;
				}
			}

		};

		template < typename D > class dc 
			: public  block_t < mexo::ps::config_s >, public machine<D,signal_t> {
		public:
			typedef  mexo::ps::config_s config_s;
			typedef  machine<D, signal_t> machine;
			typedef  block_t < config_s> block;
		public:
			dc(subsystem& _subsystem, cstr  _name, config_s & _config)
				: block(_subsystem, _name, _config)
				, machine(power :: desired)
			{

			}
 			
			virtual bool applay(const config_s & _config) {
				rampGain	=	_config.rampGain;
				machine::enable();
				return true;
			}

			virtual void execute (void) {
				if (power::active()) {
					power::ramp();
				}
				machine::execute();
			}			
		};

		class dev : public node {
		public:
			struct action {
				bool actual;
				int mode = idle_id;
			} & action_ref;
			struct snapshot {
				int mode = idle_id;
			} & snapshot_ref;

			ps::power  & power;
			enum { idle_id = 0 };
			
			class mode : public subsystem {
				friend class dev;
			public:
				typedef robo::list::unique<mode, int> map;
				typedef map::ref ref;
			private:
				ref ref_;
			protected:				
				dev& owner() { return *((dev*)node::owner()); };
				virtual ps::power::command reset(void) { return ps::power::command::on;  };
				virtual void applay_action(void) { };
			public:
				mode(int _index, cstr  _name, dev& _dev);
			};

			class idle_mode : public mode {
			protected:
				virtual ::mexo::ps::power::command reset(void) { return ::mexo::ps::power::command::on; };
			public:
				idle_mode(dev& _dev) : ::mexo::ps::dev::mode( idle_id, RT("idle"), _dev ) {};
			};

			idle_mode idle;
			dev(cstr  _name, ps::power& _power, action& _action, snapshot& _snapshot);
			void switch_to(int _mode);
			//void switch_to(mode & _mode);

			virtual void do_enable(void);
			virtual void do_disable(void);



		private:
			friend class mode;
			mode::map modes_;
			mode* actual_mode_;
			int actual_mode_id_;
			//::robo::delegat::member< mexo::machine::slot::delegat, dev, void> frontend_;
			::robo::delegat::member< mexo::machine::slot::delegat, dev, void> backend_;
			//::mexo::machine::slot::delegat::ref  frontend_ref_;
			::mexo::machine::slot::delegat::ref  backend_ref_;
			void backend__(void) {
				robo::system::guard g__;
				if (action_ref.mode != actual_mode_id_) {
					switch_to(action_ref.mode);
				} else{
					if (action_ref.actual) {
						action_ref.actual = false;
						actual_mode_->applay_action();
					}
				}
			}
			//void frontend__(void) {	}
		};

	}
}
#endif