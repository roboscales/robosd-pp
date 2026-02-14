#ifndef __roboconvert_hpp
#define __roboconvert_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_app.hpp"
namespace robo{

#if		ROBO_APP_MODULE_ENABLED == 1
	
	class ROBO_EXPORT converter: public app::node{
	private:
		float offset_ = 0.f;
		float scale_ = 1.f;
		float min_ = 0.f;
		float max_ = 0.f;
		template< typename T> T sut_(T value) const {
			if (value > T(max_)) value = T(max_);
			else if (value < T(min_)) value = T(min_);
			return value;
		}

		float eps_ = 0.f;
	public:
		float offset(void) { return offset_;};
		float scale(void){ return scale_;}
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
		float min(void){ return min_;}
		float max(void){ return max_;}
		float eps(void){ return eps_;}
		int8_t to_i8(float _value) const;
		int16_t to_i16(float _value)  const;
		int32_t to_i24(float _value) const;
		template<typename T> int32_t to_i32(T value) const {
			T tmp = sut_(value);
			tmp = ((tmp - offset_) * scale_);
			if (tmp > T(0.)) tmp += T(0.5);
			if (tmp < T(0.)) tmp -= T(0.5);
			if (tmp < T(-2147483647.0)) tmp = T(-2147483647.0);
			else if (tmp > T(2147483647.0)) tmp = T(2147483647.0);
			return (int32_t)tmp;
		}



		uint8_t to_u8(float _value) const;
		uint16_t to_u16(float _value) const;
		uint32_t to_u24(float _value) const;
		uint32_t to_u32(float _value) const;

		float to_float(int _value) const;
		double to_double(int _value) const;
		converter( cstr _name, app::node * _owner );
		static converter * find(cstr _name);
	protected:
		virtual bool do_load(void);
	};
#endif
}

#endif
