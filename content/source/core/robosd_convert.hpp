#ifndef __roboconvert_h
#define __roboconvert_h
#include "core/robosd_list.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_app.hpp"
namespace robo{
	class ROBO_EXPORT converter: public app::node{
	private:
		float offset_;
		float scale_;
		float min_;
		float max_;
		float sut_(float value) const;
		float eps_;
	public:
		float offset(void) { return offset_;};
		float scale(void){ return scale_;}
		float min(void){ return min_;}
		float max(void){ return max_;}
		float eps(void){ return eps_;}
		int8_t to_i8(float _value) const;
		int16_t to_i16(float _value)  const;
		int32_t to_i24(float _value) const;
		int32_t to_i32(float _value) const;


		uint8_t to_u8(float _value) const;
		uint16_t to_u16(float _value) const;
		uint32_t to_u24(float _value) const;
		uint32_t to_u32(float _value) const;

		float to_float(int _value) const;
		converter( cstr _name, app::node * _owner );
		static converter * find(cstr _name);
	protected:
		virtual bool do_load(void);
	};
}

#endif
