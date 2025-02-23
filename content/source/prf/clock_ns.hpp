#ifndef clock_ms_hpp
#define clock_ms_hpp
#include <stdint.h>
namespace robo{
	namespace prf{
		template < typename P> class clock32_drv_t: public P{
			public:
				typedef typename P::ns_t ns_t ; 
				typedef typename P::us_t us_t ; 
				typedef typename P::tick_t tick_t ; 

				static tick_t ns2tick(ns_t _ns){
					return (tick_t)(( (uint64_t)536870912 + (uint64_t)(1.073741824*P::clock_hz()) *_ns )>>30);
				}
		
				static ns_t tick2ns(tick_t _tick){
					return (ns_t)(((uint64_t)((uint64_t)1024*1e9 / P::clock_hz()) *_tick +512) >> 10);
				}
				
				static tick_t us2tick(us_t _us){
					return (tick_t)(( (uint64_t)524288 + (uint64_t)(P::clock_hz()*1.048576) *_us )>>20);
				}
		
				static us_t tick2us(tick_t _tick){
					return (us_t)(( (uint64_t)((uint64_t)1024*1024*1e6 / P::clock_hz()) *_tick +524288) >> 20);
				}
				static  tick_t tick(void){
					return (tick_t)P::tick();
				}
		};
		template <class D > class clock_ns_t{
			public: 
				typedef typename  D::tick_t tick_t;
				typedef typename  D::ns_t ns_t;
				typedef typename  D::us_t us_t;
			private:
				tick_t tick_ = 0;
			public: 
				clock_ns_t(void){tick_ = D::tick();}
				void tick(void){ tick_ = D::tick(); }
				ns_t tock(void){ return D::tick2ns(D::tick() - tick_); }
				static ns_t ns(void){
					return D::tick2ns( D::tick());
				}
				static void delay_ns(ns_t _ns){
					volatile tick_t begin = D::tick();
					volatile tick_t period = D::ns2tick(_ns);
					while(D::tick() - begin < period ) D::idle();
				}
				static us_t us(void){
					return D::tick2us( D::tick());
				}
				static void delay_us(ns_t _us){
					volatile tick_t begin = D::tick();
					volatile tick_t period = D::us2tick(_us);
					while(D::tick() - begin < period ) D::idle();
				}
				
		};
		
		
	}
}
#endif