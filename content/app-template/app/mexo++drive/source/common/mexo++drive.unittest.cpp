#include "mexo++drive.hpp"
#if MEXO_DRIVE_NUMBER_TEST_ENABLED
namespace number_test {

	template<typename T> struct real{
		struct {
			T value = T(0);
		} sss;
		volatile real & operator = (const real & _src){
			sss.value = _src.sss.value;
			return * this;
		}
		real(const real & _value){sss.value =_value.sss.value;}
		template <typename S> real(const S & _value){sss.value =_value;}
		
		template <typename S> real & operator = (const S & _src) {
			sss.value = _src;
			return *this;
		}
		void operator *= (const real & _src) {
			sss.value *= _src.sss.value;
		}	
		void operator += (const real & _src) {
			sss.value += _src.sss.value;
		}	
		template <typename S> void operator *= (const S & _src){
			sss.value *= _src;
		}
		template <typename S> void operator += (const S & _src){
			sss.value += _src;
		}
	};
	template<typename T>   real<T> operator * (const real<T> & _x, const real<T> & _y) {
		real<T> tmp(_x);
		tmp*=_y;
		return tmp;
	}	
	template<typename T>   real<T> operator + (const real<T> & _x, const real<T> & _y) {
		real<T> tmp(_x);
		tmp+=_y;
		return tmp;
	}	
	template<typename T> struct rreal{
		T & value;
		rreal & operator = (const rreal & _src){
			value = _src.value;
			return * this;
		}
		rreal(T & _value)
			: value (_value){}
		
		T dummy;
		rreal(const T & _value)
			: value (dummy){ dummy = _value; }
		rreal()
			: value (dummy){ dummy = 0; }

		template <typename S> rreal & operator = (const S & _src){
			value = _src;
			return *this;
		}
		
		rreal & operator *= (const rreal & _src){
			value *= _src.value;
			return *this;
		}

		rreal & operator += (const rreal & _src){
			value *= _src.value;
			return *this;
		}

		template <typename S> rreal & operator *= (const S & _src){
			value *= _src;
			return *this;
		}

	};
	
	template<typename R, typename X, typename Y> struct smult {
		R  r = 1;
		mppd::clock::us_t ns;
		void run( ) {	
			mppd::clock cl;
			cl.tick();
			X x(1);
			Y y(1);
			for (int i=0;i<10;++i){
				x = mppd::clock::prf::tick();
				y = mppd::clock::prf::tick();
				for(int j=0;j<100;j++){
					r *= x;
					r *= y;
					r += x;
					r += y;
				}
			}
			ns = (cl.tock()+512);
		}
		
	}	;
	struct res{
		int res;
		mppd::clock::us_t ns;
	};
	slot::simple start(
		slot::kind::start
		, []{
			::mexo::tp.set_verb(111);
		}
	);
		
	slot::simple frontend(
		slot::kind::frontend
		, []{
			static smult<int,int,int> smult_32_32_32;

			::mexo::tp.on(111);
			smult_32_32_32.run();
			::mexo::tp.off(111);
			mppd::clock::delay_us(400);
			
			volatile static res  smult_32_32_32res;
			smult_32_32_32res.res = smult_32_32_32.r;
			smult_32_32_32res.ns  = smult_32_32_32.ns;

			static smult<real<int>,real<int>,real<int>> csmult_32_32_32;
			::mexo::tp.on(111);
			csmult_32_32_32.run();
			::mexo::tp.off(111);

			mppd::clock::delay_us(400);

			volatile static res  csmult_32_32_32res;
			csmult_32_32_32res.res = csmult_32_32_32.r.sss.value;
			csmult_32_32_32res.ns  = csmult_32_32_32.ns;
			
			
			static smult<rreal<int>,rreal<int>,rreal<int>> rsmult_32_32_32;
			::mexo::tp.on(111);
			rsmult_32_32_32.run();
			::mexo::tp.off(111);
			volatile static res  rsmult_32_32_32res;
			rsmult_32_32_32res.res = rsmult_32_32_32.r.value;
			rsmult_32_32_32res.ns  = rsmult_32_32_32.ns;

			mppd::clock::delay_us(1200);

		}
	);		
}
#endif