#include "core/robosd_log.hpp"
#include <stdio.h>

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


#if ROBO_TERMINAL_PRINT_ENABLED == 1
	namespace termial{
		typedef enum
		{
			Black = 0,
			Blue = 1,
			Green = 2,
			Cyan = 3,
			Red = 4,
			Magenta = 5,
			Brown = 6,
			LightGray = 7,
			DarkGray = 8,
			LightBlue = 9,
			LightGreen = 10,
			LightCyan = 11,
			LightRed = 12,
			LightMagenta = 13,
			Yellow = 14,
			White = 15
		} consol_color_t;

		static cstr consol_colors_[16] = {
			RT("\x1B[30m"),
			RT("\x1B[34m"),
			RT("\x1B[32m"),
			RT("\x1B[36m"),
			RT("\x1B[31m"),
			RT("\x1B[35m"),
			RT("\x1B[30m"),
			RT("\x1B[37m"),
			RT("\x1B[37m"),
			RT("\x1B[34m"),
			RT("\x1B[32m"),
			RT("\x1B[36m"),
			RT("\x1B[31m"),
			RT("\x1B[35m"),
			RT("\x1B[33m"),
			RT("\x1B[37m")
		};

		static cstr background_colors_[16] = {
				RT("\x1B[40m"), //Black = 0,
				RT("\x1B[44m"), //Blue = 1,
				RT("\x1B[42m"), //Green = 2,
				RT("\x1B[46m"), //Cyan = 3,
				RT("\x1B[41m"), //Red = 4,
				RT("\x1B[45m"), //Magenta = 5,
				RT("\x1B[40m"), //Brown = 6,
				RT("\x1B[47m"), //LightGray = 7,
				RT("\x1B[47m"), //DarkGray = 8,
				RT("\x1B[44m"), //LightBlue = 9,
				RT("\x1B[42m"), //LightGreen = 10,
				RT("\x1B[46m"), //LightCyan = 11,
				RT("\x1B[41m"), //LightRed = 12,
				RT("\x1B[45m"), //LightMagenta = 13,
				RT("\x1B[43m"), //Yellow = 14,
				RT("\x1B[47m") //White = 15
		};

		void set_consol_color_(consol_color_t text, consol_color_t background)
		{
#if ROBO_UNICODE_ENABLED ==1
				wprintf( consol_colors_[text] );
				wprintf( background_colors_[background]);
#else
				printf(  consol_colors_[text] );
				printf( background_colors_[background]);
#endif			
		}

		void print( robo::log::verb _verb,  cstr _format, va_list  _args){
			switch (_verb){
			case robo::log::verb::error:
				set_consol_color_(LightRed, Black);
				break;
			case robo::log::verb::warning:
				set_consol_color_(Yellow, Black);
				break;
			case robo::log::verb::info:
				set_consol_color_(LightGreen, Black);
				break;
			case robo::log::verb::detail_1:
				set_consol_color_(LightCyan, Black);
				break;
			case robo::log::verb::detail_2:
				set_consol_color_(LightMagenta, Black);
				break;
			case robo::log::verb::detail_3:
				set_consol_color_(LightGreen, Blue);
				break;
			case robo::log::verb::detail_4:
				set_consol_color_(LightMagenta, Blue);
				break;
			case robo::log::verb::detail_5:
				set_consol_color_(LightCyan, Blue);
				break;
			case robo::log::verb::detail_6:
				set_consol_color_(White, Blue);
				break;
			case robo::log::verb::detail_7:
				set_consol_color_(White, Blue);
				break;
			default:
				set_consol_color_((consol_color_t)( (int)_verb & 0xF), (consol_color_t)(( (int)_verb & 0xF0) >> 4) );
			}
#if ROBO_UNICODE_ENABLED ==1
			vwprintf(_format, _args);
#else
			vprintf(_format, _args);
#endif
			printf("\n\r");
			set_consol_color_(White, Black);
		}
	}
#endif		
#endif	
}
