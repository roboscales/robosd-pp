#include "mexo++drive.hpp"
#include "mexo++\math.hpp"
#if MPPD_DRIVE_NUMBER_TEST_ENABLED == 1
namespace number_test {
	template<template<class> class N,typename T>   T operator * (const N<T> & _x, const N<T> & _y) {
		return _x.value*_y.value;
	}		
	template<template<class> class N,typename T>   T operator * (const T & _x, const N<T> & _y) {
		return _x*_y.value;
	}	
	template<template<class> class N, typename T>   T operator * (const N<T> & _x, const T & _y) {
		return _x.value*_y;
	}	
	template<template<class> class N,typename T>   T operator + (const N<T> & _x, const N<T> & _y) {
		return _x.value+_y.value;
	}	
	
	template<template<class> class N,typename T>   T operator + (const T & _x, const N<T> & _y) {
		return _x+_y.value;
	}	
	template<template<class> class N, typename T>   T operator + (const N<T> & _x, const T & _y) {
		return _x.value+_y;
	}		
	template<typename T> struct real{
		T value = T(0);
		volatile real & operator = (const real & _src){
			value = _src.value;
			return * this;
		}
		real(const real & _value){value =_value.value;}
		template <typename S> real(const S & _value){value =_value;}
		
		template <typename S> real & operator = (const S & _src) {
			value = _src;
			return *this;
		}
		void operator *= (const real & _src) {
			value *= _src.value;
		}	
		void operator += (const real & _src) {
			value += _src.value;
		}	
		template <typename S> void operator *= (const S & _src){
			value *= _src;
		}
		template <typename S> void operator += (const S & _src){
			value += _src;
		}
	};
	
	template<typename T> struct preal{
		T * value = (T *)0;
		preal & operator = (const preal & _src){
			value = _src.value;
			return * this;
		}
		preal(T & _value)
			: value (&_value){}
		
		T dummy;
		preal(const T & _value)
			: value (&dummy){ dummy = _value; }
		
			preal()
			: value (dummy){ dummy = 0; }
		

		preal(const preal & _value){*value =*_value.value;}
		template <typename S> preal(const S & _value){*value =_value;}
		
		template <typename S> preal & operator = (const S & _src) {
			*value = _src;
			return *this;
		}
		void operator *= (const preal & _src) {
			*value *=* _src.value;
		}	
		void operator += (const preal & _src) {
			*value += *_src.value;
		}	
		template <typename S> void operator *= (const S & _src){
			*value *= _src;
		}
		template <typename S> void operator += (const S & _src){
			*value += _src;
		}
	};
	template<typename T>   T operator * (const preal<T> & _x, const preal<T> & _y) {
		return *_x.value *  *_y.value;
	}		
	template<typename T>   T operator * (const T & _x, const preal<T> & _y) {
		return _x * *_y.value;
	}	
	template<typename T>   T operator * (const preal<T> & _x, const T & _y) {
		return *_x.value * _y;
	}	
	template<typename T>   T operator + (const preal<T> & _x, const preal<T> & _y) {
		return *_x.value + *_y.value;
	}	
	
	template<typename T>   T operator + (const T & _x, const preal<T> & _y) {
		return _x + *_y.value;
	}	
	template<typename T>   T operator + (const preal<T> & _x, const T & _y) {
		return *_x.value + _y;
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
			value += _src.value;
			return *this;
		}

		template <typename S> rreal & operator *= (const S & _src){
			value *= _src;
			return *this;
		}

	};
	volatile int r2=0;
	int x2=0;
	int y2=0;
	volatile int& rr2= r2;
	int& rx2  = x2;
	int& ry2= y2;
	int xx = 0;
	int yy = 0;
	int rr = 0;
		
	template<typename R, typename X, typename Y> struct smult {
		R r;
		X x;
		Y y;
		smult(): r(rr),x(xx),y(yy){}
		mppd::clock::ns_t ns;
		void run( ) {	
			mppd::clock cl;
			cl.tick();
			for (int i=0;i<100;++i){
				x = mppd::clock::prf::tick();
				for(int j=0;j<100;j++){
					y = mppd::clock::prf::tick();
			//		mppd::clock c2;
					//c2.tick();
					int tmp = x+y;
					tmp += (x*y);
					r += tmp;// + (x+y);
					//r+= (x+y);
//					ns += c2.tock();
//					r *= y;
//					r += x;
//					r += y;
				}
			}
			ns = (cl.tock());
		}
		
	}	;
	struct res{
		int res;
		mppd::clock::ns_t ns;
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
			csmult_32_32_32res.res = csmult_32_32_32.r.value;
			csmult_32_32_32res.ns  = csmult_32_32_32.ns;
			
			
			static smult<rreal<int>,rreal<int>,rreal<int>> rsmult_32_32_32;
			::mexo::tp.on(111);
			rsmult_32_32_32.run();
			::mexo::tp.off(111);
			volatile static res  rsmult_32_32_32res;
			rsmult_32_32_32res.res = rsmult_32_32_32.r.value;
			rsmult_32_32_32res.ns  = rsmult_32_32_32.ns;

			static smult<preal<int>,preal<int>,preal<int>> psmult_32_32_32;
			::mexo::tp.on(111);
			psmult_32_32_32.run();
			::mexo::tp.off(111);
			volatile static res  psmult_32_32_32res;
			psmult_32_32_32res.res = *psmult_32_32_32.r.value;
			psmult_32_32_32res.ns  = psmult_32_32_32.ns;
			x2= mppd::clock::prf::tick();
			y2= mppd::clock::prf::tick();
			r2 = x2+y2;
			x2= mppd::clock::prf::tick();
			y2= mppd::clock::prf::tick();
			rr2 = rx2+ry2;
			mppd::clock::delay_us(1200);

			using math = ::mexo::math<::mexo::digit15> ;
			math::unit_test::run();
			//math::number_t a
			//precise_round

		}
	);		
}
#endif