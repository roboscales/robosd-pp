#include "core/robosd_convert.hpp"
#include "core/robosd_ini.hpp"
namespace robo{
#if		ROBO_APP_MODULE_ENABLED == 1
	const char * convert_names[] = {
		"Смещение",
		"Разр.",
		"Мин.",
		"Макс.",
		0
	};
	bool converter::do_load(void){
		robo::string key;
		ROBO_LBREAKN(ini::load(current_path(), defaults_path(), RT("offset"),offset_));
		ROBO_LBREAKN(ini::load(current_path(), defaults_path(), RT("scale"),scale_));
		ROBO_LBREAKN(ini::load(current_path(), defaults_path(), RT("min"),min_));
		ROBO_LBREAKN(ini::load(current_path(), defaults_path(), RT("max"),max_));
		eps_ = abs(0.5f/scale_);
		return true;
	}
	converter::converter(cstr _name, app::node * _owner ): app::node(_name,_owner){
	}

	converter * converter::find(cstr _name){
		return dynamic_cast<converter*>(app::node::find(_name));
	}

	int8_t converter::to_i8(float value) const{
		float tmp = sut_(value);
		tmp = ((tmp - offset_)* scale_);
		if (tmp>0.f) tmp += 0.5f;
		if (tmp<0.f) tmp -= 0.5f;
		if (tmp<-127.0) tmp = -127.0f;
		else if (tmp>127.0f) tmp = 127.0f;
		return (int8_t)tmp;
	}

	int16_t converter::to_i16(float value) const{
		float tmp = sut_(value);
		tmp = ((tmp - offset_)* scale_);
		if (tmp>0.f) tmp += 0.5f;
		if (tmp<0.f) tmp -= 0.5f;
		if (tmp<-32767.0f) tmp = -32767.0f;
		else if (tmp>32767.0f) tmp = 32767.0f;
		return (int16_t)tmp;
	}

	int32_t converter::to_i24(float value) const{
		float tmp = sut_(value);
		tmp = ((tmp - offset_)* scale_);
		if (tmp>0.f) tmp += 0.5f;
		if (tmp<0.f) tmp -= 0.5f;
		if (tmp<-8388607.0f) tmp = -8388607.0f;
		else if (tmp>8388607.0f) tmp = 8388607.0f;
		return (int32_t)tmp;
	}


	uint8_t converter::to_u8(float value) const{
		float tmp = sut_(value);
		tmp = ((tmp - offset_)* scale_);
		if (tmp>0.f) tmp += 0.5f;
		if (tmp<0.0f) tmp = 0.0f;
		else if (tmp>255.0f) tmp = 255.0f;
		return (int8_t)tmp;
	}

	uint16_t converter::to_u16(float value) const{
		float tmp = sut_(value);
		tmp = ((tmp - offset_)* scale_);
		if (tmp > 0.f) tmp += 0.5f;
		if (tmp<0.0f) tmp = 0.0f;
		else if (tmp>65537.0f) tmp = 65537.0f;
		return (int16_t)tmp;
	}

	uint32_t converter::to_u24(float value) const{
		float tmp = sut_(value);
		tmp = ((tmp - offset_)* scale_);
		if (tmp > 0.f) tmp += 0.5f;
		if (tmp<0.0f) tmp = 0.0f;
		else if (tmp>16777215.0f) tmp = 16777215.0f;
		return (int32_t)tmp;
	}

	uint32_t converter::to_u32(float value) const{
		float tmp = sut_(value);
		tmp = ((tmp - offset_)* scale_);
		if (tmp > 0.f) tmp += 0.5f;
		if (tmp<0.0f) tmp = 0.0f;
		else if (tmp>4294967295.0f) tmp = 4294967295.0f;
		return (int32_t)tmp;
	}

	float converter::to_float(int value) const{
		if (scale_)
			return  (float)value / scale_ + offset_;
		else
			return 0.f;
	}

	double converter::to_double(int value) const {
		if (scale_)
			return  (double)value / scale_ + offset_;
		else
			return 0.f;
	}

#endif	
}
