#include "core/robosd_log.hpp"


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
namespace robo{
	namespace log{
		print_f print_ = 0;

		verb verb_ = verb::info;
		unsigned int mask_ = 0;

		void  print(verb _verb, unsigned int _mask, cstr _format, ...){
			if (
				(print_ != 0) && 
			(
				(_verb < verb::info) || 
				((verb_>= _verb) && ((mask_ & _mask) == _mask))
			)) 
			{
				va_list args;
				va_start(args, _format);
				print_(_verb, _format, args);
				va_end(args);
			}
		}

		bool begin(verb _verb, unsigned int _mask, print_f _print){
			verb_ = _verb;
			mask_ = _mask;
			print_ = _print;
			return true;
		}

		void finish(void){
			print_ = nullptr;
		}
	}
}

#endif
