#ifndef robosd_net_modbus_hpp
#define robosd_net_modbus_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_system.hpp"
#include "net/robosd_net_trafic.hpp"
#include "core/robosd_autonum.hpp"
#include <algorithm>
namespace robo{
	namespace net{
		namespace modbus{
			template < class M , class G > class dispetcher_t : public M{
				public:
					typedef typename  M::errors errors;
					class regs;
					friend class regs;
					friend class device_c;
					class device_c{
					public:
					friend class regs;
					protected:
						robo::time_us_t timeout_us = 0;
						robo::time_us_t last_request_us = 0;
						//robo::time_us_t timeout;
						void on_request(void){
							last_request_us = robo::system::time_us();
						}
						virtual bool exchange_need(void){
							return  (robo::system::time_us() - last_request_us > timeout_us);
						}
					public:
						uint8_t devaddr; //Адрес устройства
						~device_c(void) { }
						device_c(
							uint8_t _devaddr
							, robo::time_us_t _timeout_us				
						)
						: devaddr(_devaddr)
						, timeout_us(_timeout_us)
						{
						}
					};
					
					class regs{
					public:
						typedef ::robo::list::unsorted<regs> list;
						typedef typename list::ref ref;
					protected:
						bool check_prev;
						device_c & device;
						dispetcher_t & dispetcher;
						uint16_t regaddr; //Адрес первого регистра 
						uint16_t count; //Количество регистров
						uint16_t * memo;
						uint16_t * prev = nullptr;
						void set ( uint16_t * _memo){ 
							G g__;
							if(memo)
								std::copy_n(memo,count,_memo); 
						}
						void get ( const uint16_t * _memo){ 
							G g__;
							if(memo)
								std::copy_n(_memo,count,memo); 
						}
					private:
						ref ref_;
						bool active_;
						bool continues_;
						bool first_;
					protected:
						virtual void on_request(void) = 0;
						virtual void on_confirm(void) = 0;
						virtual void on_refuse(const errors & _err) = 0;
					public:
						bool active(void) { return active_; }
						void resume(void) { active_ = true; continues_ = true;};
						void pulse(void) { active_ = true; continues_ = false; };
						void pause(void){active_ = false;};
						statistic_s<errors> statistic = {};
						void request(void){
							on_request();
							device.on_request();
							statistic.request ++;
						}
						void confirm(void){
							on_confirm();
							statistic.confirm ++;
							if (!continues_) { active_ = false; }
						}
						void refuse(const errors& _err){
							on_refuse(_err);
							statistic.refuse.total ++;
							statistic.refuse.detail[(int)_err] ++;
						}
						virtual bool exchange_need(void){
							return device.exchange_need() && active_;
						}
						enum class actives { on,off};
						enum class continues { on, off };
						enum class freshes { on, off };
						regs(
							dispetcher_t & _dispetcher
							, device_c & _device
							, uint16_t _regaddr
							, uint16_t _count
							, actives _active
							, continues _continues
							, freshes _check_prev
						)
						: ref_(*this)
						, dispetcher(_dispetcher)
						, device(_device)
						, regaddr(_regaddr)
						, count(_count)
						, continues_(_continues== continues::on)
						, active_(_active == actives::on)
						, check_prev(_check_prev == freshes::on)
						{
							if(count){
								memo =  new uint16_t[_count];
								ref_.attach_to(_dispetcher.regs_);
								if(check_prev){
									prev= new uint16_t [_count];
									std::fill_n(prev,_count,0xFFFF);
								}
							} else{
								memo =  nullptr;
							}
						}
						~regs(void) { 
							if(memo) delete[] memo; 
							if(prev) delete[] prev; 
						}
					};
				
					class outcom : public regs{
						const uint16_t * src_;
						robo::delegat::ref<void>* on_confirm_;
					public:
						outcom(
							dispetcher_t & _dispetcher
							, device_c & _device
							, uint16_t _regaddr
							, uint16_t _count
							, const uint16_t * _src
							, typename regs::actives _active
							, typename regs::continues _continues
							, typename regs::freshes _check_prev
							, robo::delegat::ref<void>* _on_confirm = nullptr
						): regs(
							_dispetcher
							, _device
							,_regaddr
							,_count
							, _active
							, _continues
							, _check_prev
						)
						, src_(_src)
						, on_confirm_(_on_confirm)
						{
						}
						void assign(void) {
							if (regs::check_prev) {
								regs::get(src_);
								std::copy_n(regs::memo, regs::count, regs::prev);
							}
						}

					protected:
						virtual void on_request(void){
							if(! regs::check_prev) {
								regs::get(src_);
							} 
					
							if(regs::count == 1){
								regs::dispetcher.write_reg(regs::device.devaddr,regs::regaddr,regs::memo[0]);
							} else {
								regs::dispetcher.write_regs(regs::device.devaddr,regs::regaddr,regs::count,regs::memo);
							}
						}
						virtual bool exchange_need(void){
							if(! regs::check_prev) {
								return regs::exchange_need();
							} else if(regs::exchange_need()) {
								regs::get(src_);
								if(  std::equal( regs::memo,  regs::memo+regs::count, regs::prev ) ){
									return false;
								} else{
									return true;
								}								
							} else{
									return false;
							}
						}
						virtual void on_confirm(void){
							if(regs::check_prev) {
								std::copy_n( regs::memo,  regs::count, regs::prev);
							}
							if (on_confirm_) {
								(*on_confirm_)();
							}
						}
						virtual void on_refuse(const errors & _err){
						}
					};
					#if ROBO_AUTONUM_ENABLED == 1
					class command : public regs{
					public:
						enum class results{ success = 1, refuse = 0};
						using confirm_s = robo::delegat::ref<void,results>;
						using autonum = robo::delegat::autonum_fabric<void,results>;
						int try_count_ = 0;
						uint16_t buf_sz_ ;
					private:
						confirm_s * on_confirm_;
					public:
						command(
							dispetcher_t & _dispetcher
							, device_c & _device
							, uint16_t _buf_sz
						): regs(
							_dispetcher
							, _device
							,0
							,_buf_sz
							, regs::actives::off
							, regs::continues::off
							, regs::freshes::off
						)
						, on_confirm_(nullptr)
						,buf_sz_(_buf_sz)
						{
						}
						void post( uint16_t _regaddr, uint16_t * _values, uint16_t _count, int _try_count = 1, confirm_s * _confirm=nullptr){
							if(_count<=buf_sz_){
								on_confirm_ = _confirm;
								regs::regaddr = _regaddr;
								regs::count = _count;
								try_count_ = _try_count;
								regs::get(_values);
								regs::pulse();
							} else {
								if (_confirm) {
									(*_confirm)(results::refuse);
								}			
							}							
						}
						template < typename S> void post( uint16_t _regaddr, const S & _s,  int _try_count = 1, confirm_s * _confirm=nullptr){
							post(_regaddr, (uint16_t *) &_s,sizeof(_s)/sizeof(uint16_t), _try_count, _confirm );
						}
						void post( uint16_t _regaddr, std::initializer_list<uint16_t>  _s,  int _try_count = 1, confirm_s * _confirm=nullptr){
							post(_regaddr, _s.begin(),_s.size(), _try_count, _confirm );
						}
					protected:
						virtual void on_request(void){					
							if(regs::count == 1){
								regs::dispetcher.write_reg(regs::device.devaddr,regs::regaddr,regs::memo[0]);
							} else {
								regs::dispetcher.write_regs(regs::device.devaddr,regs::regaddr,regs::count,regs::memo);
							}
						}
						virtual bool exchange_need(void){
							if(try_count_>0){
								return regs::exchange_need();
							} else  return false;
						}
						virtual void on_confirm(void){
							try_count_ = 0;
							if (on_confirm_) {
								(*on_confirm_)(results::success);
							}
						}
						virtual void on_refuse(const errors & _err){
							try_count_ --;
							if(try_count_<=0){
								regs::pause();
								if (on_confirm_) {
									(*on_confirm_)(results::refuse);
								}
							}
						}
					};
					#endif
					class incom : public regs{
						uint16_t * dst_;
						robo::delegat::ref<void> * on_update_;
					public:
						incom(
							dispetcher_t & _dispetcher
							, device_c & _device
							, uint16_t _regaddr
							, uint16_t _count
							, uint16_t * _dst
							, typename regs::actives _active
							, typename regs::continues _continues
							, robo::delegat::ref<void> * on_update =nullptr
						): regs(
							_dispetcher
							,_device
							,_regaddr
							,_count
							, _active
							, _continues
							, on_update !=nullptr ? regs::freshes::on : regs::freshes::off
						), dst_(_dst), on_update_(on_update){
								if(regs::check_prev) {
									regs::get(dst_);
									std::copy_n( regs::memo,  regs::count, regs::prev);
								}
						}
						virtual void on_request(void){
							/*
							if(regs::count == 1){
								regs::dispetcher.read_reg(regs::device.devaddr,regs::regaddr,regs::memo[0]);
							} else {
								regs::dispetcher.read_regs(regs::device.devaddr,regs::regaddr,regs::count,regs::memo);
							}
							*/
							
							regs::dispetcher.read_regs(regs::device.devaddr,regs::regaddr,regs::count,regs::memo);
						}
						bool first = false;
						void first_update() { first = true; }
						virtual void on_confirm(void){
							regs::set(dst_);
							if(regs::check_prev ){
								if( first || ! std::equal( regs::memo,  regs::memo+regs::count, regs::prev ) ){
									first = false;
									std::copy_n( regs::memo,  regs::count, regs::prev);
									if(on_update_) (* on_update_)();
								}
							}
						}
						virtual void on_refuse(const errors & _err){
						}
					};
				private:
					typename regs::list regs_;
					typename regs::ref * current_ = nullptr;
					typename regs::ref * active_ = nullptr;
				protected:
				
					virtual void dispetcher_confirm(void){
						if(active_ != nullptr){
							active_->owner().confirm();
							active_ = nullptr;
						}
					}

					virtual void dispetcher_refuse(const errors & _err){
						if(active_ != nullptr){
							active_->owner().refuse(_err);
							active_ = nullptr;
						}
					}
					virtual bool dispetcher_ready(void){
						 return (active_ == nullptr);
					}
					virtual bool dispetcher_request(void){

							if (current_) {
								current_ = current_->next();
							}

							if (current_ == nullptr) {
								current_ = regs_.first();
							}

							if (current_) {
								if (current_->owner().exchange_need() ) {
									robo::system::critical g__;
									active_ = current_;
									active_->owner().request();
									return true;
								}
							}
						return false;
					}
				public:
					dispetcher_t(robo::time_us_t _timeout_us){
						 M::timeout_us = _timeout_us;
					}
			};
			
			#if 0
			class entry;
			class phy{				
				friend class entry;
				public:
				typedef ::robo::list::unique<phy, int> map;
				typedef map::ref ref;
				private:
				entry * entry_;
				ref ref_;
				static map & map_(void);
				static phy * find_(cstr _alias);
				
				static phy * dummy_(void);
				phy(void);
				
				public:
					phy(cstr _alias);
				public:
					void onReceive(const uint8_t * _packet, size_t _size);
					void onSend(void);
					void onSendError(void);
					void onReceiveError(void);
				protected:
					virtual bool startReceive()=0;
					virtual bool startSend(const uint8_t * _packet, size_t _size)=0;
			};
			
			class entry{
				friend class phy;
				phy * phy_;
				static entry * dummy_(void);
				entry(phy * _dummy);
				public:
					entry(void);
					bool start(cstr _phys);				
					void onReceive(const uint8_t * _packet, size_t _size);
					void onSend(void);
					void onSendError(void);
					void onReceiveError(void);
					virtual bool startReceive(void){ return phy_->startReceive();}
					virtual bool startSend(const uint8_t * _packet, size_t _size){ return phy_->startSend(_packet,_size);};
			};
			#endif
		}
	}
}
#endif