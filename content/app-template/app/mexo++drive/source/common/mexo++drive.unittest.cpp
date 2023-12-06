#include "mexo++drive.hpp"
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
		mppd::clock::ns_t ns;
		void run( ) {	
			mppd::clock cl;
			cl.tick();
			X x(1);
			Y y(1);
			for (int i=0;i<100;++i){
				for(int j=0;j<100;j++){
					x = mppd::clock::prf::tick();
					y = mppd::clock::prf::tick();
					r += x*y;// + (x+y);
					r+= (x+y);
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

			mppd::clock::delay_us(1200);

		}
	);		
}
#endif