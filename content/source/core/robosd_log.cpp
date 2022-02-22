#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_string.hpp"
#include <stdio.h>

#if ROBO_UNICODE_ENABLED == 1
#include <wchar.h>
#endif

namespace robo {
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
    namespace log {
        verb verb_ = verb::info;
        unsigned int mask_ = 0;

        void print(verb _verb, unsigned int _mask, cstr _format, ...) {
						#if ROBO_APP_PRINT_TYPE != ROBO_APP_TYPE_NONE
            if ( (_verb < verb::info) ||  ((verb_ >= _verb) && ((mask_ & _mask) == _mask)) )
            {
                va_list args;
                va_start(args, _format);
                system::env::print(_verb, _format, args);
                va_end(args);
            }
						#endif
        }

        bool begin(verb _verb, unsigned int _mask) {
            verb_ = _verb;
            mask_ = _mask;
            return true;
        }

        void finish(void) {
        }
    }
#endif
}
