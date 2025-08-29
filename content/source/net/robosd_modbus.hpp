#ifndef robosd_net_modbus_hpp
#define robosd_net_modbus_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_system.hpp"
#include "net/robosd_net_trafic.hpp"
namespace robo{
	namespace net{
		namespace modbus{
			template < class M > class dispetcher_t : public M{
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
							return  true; // (robo::system::time_us() - last_request_us > timeout_us);
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
						device_c & device;
						dispetcher_t & dispetcher;
						uint16_t regaddr; //Адрес первого регистра 
						uint16_t count; //Количество регистров
						uint16_t * memo;
						void set ( uint16_t * _memo){ 
							robo::system::guard g__;
							std::copy_n(memo,count,_memo); 
						}
						void get ( const uint16_t * _memo){ 
							robo::system::guard g__;
							std::copy_n(_memo,count,memo); 
						}
					private:
						ref ref_;
						bool active_;
						bool continues_;
					protected:
						virtual void on_request(void) = 0;
						virtual void on_confirm(void) = 0;
						virtual void on_refuse(const errors & _err) = 0;
					public:
						bool active(void) { return active_; }
						void activate(void) { active_ = true; };
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

						regs(
							dispetcher_t & _dispetcher
							, device_c & _device
							, uint16_t _regaddr
							, uint16_t _count
							, bool _continues
						)
						: ref_(*this)
						, dispetcher(_dispetcher)
						, device(_device)
						, regaddr(_regaddr)
						, count(_count)
						, continues_(_continues)
						, active_(_continues)
						{
							memo =  new uint16_t[_count];
							ref_.attach_to(_dispetcher.regs_);
						}
						~regs(void) { if(memo) delete[] memo; }
					};
				
					class outcom : public regs{
						const uint16_t * src_;
					public:
						outcom(
							dispetcher_t & _dispetcher
							, device_c & _device
							, uint16_t _regaddr
							, uint16_t _count
							, const uint16_t * _src
							, bool _continues = true
						): regs(
							_dispetcher
							, _device
							,_regaddr
							,_count
							, _continues
						), src_(_src){
						}
					protected:
						virtual void on_request(void){
							regs::get(src_);
							if(regs::count == 1){
								regs::dispetcher.write_reg(regs::device.devaddr,regs::regaddr,regs::memo[0]);
							} else {
								regs::dispetcher.write_regs(regs::device.devaddr,regs::regaddr,regs::count,regs::memo);
							}
						}
						virtual void on_confirm(void){
						}
						virtual void on_refuse(const errors & _err){
						}
					};
					class incom : public regs{
						uint16_t * dst_;
					public:
						incom(
							dispetcher_t & _dispetcher
							, device_c & _device
							, uint16_t _regaddr
							, uint16_t _count
							, uint16_t * _dst
							, bool _continues = true
						): regs(
							_dispetcher
							,_device
							,_regaddr
							,_count, _continues), dst_(_dst){
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
						virtual void on_confirm(void){
							regs::set(dst_);
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