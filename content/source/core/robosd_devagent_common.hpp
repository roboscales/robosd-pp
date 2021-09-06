/**
 * @file robosd_devagent_common.hpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2021-09-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef robosd_devagent_common_hpp
#define robosd_devagent_common_hpp
namespace robo {
	namespace common {
		struct idevagent {
			enum  icommand {
				stop = 0,
				sw2service = 1,
				raise_fault = 2,
				sw2independed = 3,
				sw2dirrect = 4,
				reset_fault = 5
			};
			struct istate {
				enum class ilocal {
					unknown = 0,
					disabled = 1,
					configure = 2,
					ready = 3
				} local = ilocal::unknown;

				enum class iremote {
					unknown = 0,
					stopped = 1,
					fault = 2,
					run = 3,
					broke = 4
				} remote = iremote::unknown;
			};
			enum class istatus {
				unknown = 0,
				disabled = 1,
				busy = 2,
				stopped = 3,
				fault = 4,
				dirrect = 5,
				independed = 6,
				service = 7,
				broke = 8
			};
		};
	}
}
#endif