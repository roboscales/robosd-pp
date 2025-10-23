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
#ifndef robosd_proto_hpp
#define robosd_proto_hpp
#include <stdint.h>
namespace robo {
	namespace common {
		namespace devagent {
			//команды агенту исполнительного устройства
			struct commands {
				enum  class locals {
					stop = 0, // стоять , выключить питание, встать на тормоз и не выполнять ни чьи команды
					sw2service = 1, // переключиться в сервисный режим. С этого момента команды на устройство не транслируются. Оно полностью под управлением  стороннего ПО
					reset_panic = 2, //сбросить ошибку или выйти из режима busy
					sw2independed = 3, // устрйоство захватывается ядром сервера робота. Теперь только оно может управлять устройством
					sw2dirrect = 4, //устройство отдается под прямое управлдение системе управления верхнего уровня
					halt = 5 //вырубаемся, переходим в конфигурацию
				} local;
				enum  class remotes {
					stop = 0, // стоять , выключить питание, встать на тормоз и быть готовому к следующим командам
					reset_panic = 1, //сбросить ошибку
					configure = 2, //Перейцти в режим конфигурацииb
					halt = 3 //вырубаемся
				} ;
			};

			//состояние исполнительного устройства
			struct statuses {
				//состояние агента
				enum class connections {
					unknown = 0, //неясное
					disabled = 1, // анент исключен из обмена
					discovery = 2, //поиск исполнительного устройства
					lost = 3, //обмен потерян
					stable = 4, //обмен потерян
				} connection;

				enum class remotes {
					busy = 0, //состояние исполнительного устройства не соответствует требуемому
					configure = 1, //конфигурация агента и исполнительного устройства
					panic = 2, //устрйоство находится в аварии
					ready = 3, //устройство бездельничает в штатном режиме
					run = 4, //устройство работает в штатном режиме
					unknown = 5 //неясное
				} remote;

				//интегральный статус исполнительного устройства
				enum class actuals {
					disconnected = 0, // анент исключен из обмена
					busy = 1, // происходит смена состояния
					configure = 2, //
					panic = 3,//устрйоство находится в аварии
					dirrect = 4, //устройство работает под прямым управлдением системы управления верхнего уровня
					ready = 5, //устройство работает под автономным  управлдением
					independed = 6, //устройство работает под автономным  управлдением
					service = 7, //устройство работает под управлдением стороннего ПО
					unknown = 255,//неясное
				}  actual;

				static inline remotes decode(uint8_t _status) { 
					return _status <= (uint8_t)remotes::run ? (remotes)_status : remotes::unknown; }
			};

			
			struct action_s {
				commands command;
			};
			struct feedback_s {
				statuses status;
			};
			struct config_s {
			};


		}
	}
}
#endif