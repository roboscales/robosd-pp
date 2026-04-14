#ifndef burst_resolver_fusion_hpp
#define burst_resolver_fusion_hpp
#include "core/robosd_common.hpp"

#if ROBO_APP_ULTRACOMPACT == 0
#include "burst++/vartree.hpp"
#endif
#include <cstdint>
#include <cmath>
namespace burst {
	namespace resolver {

		// ============================================================================
		// Шкалы углов
		// ============================================================================

		/*! \brief Шкала в градусах с плавающей точкой. */
		struct degree_scale_s {
			using angle_t = float;
			using mangle_t = double;
			static constexpr double PERIOD = 360.0;
		};
		/*! \brief Шкала в миллиградусах (целые). */
		struct milli_degree_scale_s {
			using angle_t = int32_t;
			using mangle_t = int32_t;
			static constexpr int32_t PERIOD = 360000;
		};

		/*! \brief Шкала в отсчётах на 2π (65536 = 2^16). */
		struct frac_pi_scale_s {
			using angle_t = int32_t;
			using mangle_t = int32_t;
			static constexpr int32_t PERIOD = 65536;
		};

		// ============================================================================
		// Драйвер для плавающей арифметики
		// ============================================================================

		template<typename Scale, typename Params>
		class float_driver_t {
		public:
			using angle_t = typename Scale::angle_t;
			using mangle_t = typename Scale::mangle_t;
			#if ROBO_APP_BURST_VARTREE_ENABLED
			static constexpr auto angle_var = Scale::angle_var;
			static constexpr auto mangle_var = Scale::mangle_var;
			#endif
			static constexpr mangle_t PERIOD = Scale::PERIOD;
			static constexpr mangle_t HALF_PERIOD = PERIOD / 2;

			static constexpr mangle_t P_GROSS = Params::P_GROSS;
			static constexpr mangle_t P_FINE  = Params::P_FINE;
			static constexpr mangle_t MAX_ACTUATOR_ANGLE = Params::MAX_ACTUATOR_ANGLE;
			static constexpr mangle_t REDUCTION = Params::REDUCTION;
			static constexpr mangle_t D_THRESH = Params::D_THRESH;
			static constexpr mangle_t D_ABS_THRESH = Params::D_ABS_THRESH;

			static constexpr mangle_t SECTOR_DEG = PERIOD / P_GROSS;
			static constexpr mangle_t SECTOR_SIZE_ABS = SECTOR_DEG / REDUCTION;

			static mangle_t to_mangle(angle_t x) { return static_cast<mangle_t>(x); }

			static mangle_t normalise(mangle_t angle) {
				angle = std::fmod(angle, PERIOD);
				if (angle < 0) angle += PERIOD;
				return angle;
			}

			static mangle_t short_diff(mangle_t a, mangle_t b) {
				mangle_t d = a - b;
				d = std::fmod(d, PERIOD);
				if (d > HALF_PERIOD) d -= PERIOD;
				if (d < -HALF_PERIOD) d += PERIOD;
				return d;
			}

			static mangle_t abs(mangle_t x) { return (x < 0) ? -x : x; }

			static mangle_t to_actuator(mangle_t angle_abs) { return angle_abs * REDUCTION; }
			static mangle_t to_abs_sensor(mangle_t angle_actuator) { return angle_actuator / REDUCTION; }

			static mangle_t to_central_sector(mangle_t a) {
				return static_cast<mangle_t>(std::floor(a / SECTOR_SIZE_ABS));
			}

			static mangle_t to_electrical(mangle_t mech_angle, mangle_t P) {
				return normalise(mech_angle * P);
			}
		};

		// ============================================================================
		// Драйвер для целочисленной арифметики
		// ============================================================================

		template<typename Scale, typename Params>
		class int_driver_t {
		public:
			using angle_t = typename Scale::angle_t;
			using mangle_t = typename Scale::mangle_t;
			#if ROBO_APP_BURST_VARTREE_ENABLED
			static constexpr auto angle_var = Scale::angle_var;
			static constexpr auto mangle_var = Scale::mangle_var;
			#endif
			static constexpr mangle_t PERIOD = Scale::PERIOD;
			static constexpr mangle_t HALF_PERIOD = PERIOD / 2;

			static constexpr mangle_t P_GROSS = Params::P_GROSS;
			static constexpr mangle_t P_FINE  = Params::P_FINE;
			static constexpr mangle_t MAX_ACTUATOR_ANGLE = Params::MAX_ACTUATOR_ANGLE;
			static constexpr mangle_t D_THRESH = Params::D_THRESH;
			static constexpr mangle_t D_ABS_THRESH = Params::D_ABS_THRESH;

			static constexpr mangle_t REDUCTION_NUM = Params::REDUCTION_NUM;
			static constexpr mangle_t REDUCTION_DEN = Params::REDUCTION_DEN;

			static constexpr mangle_t SECTOR_DEG = PERIOD / P_GROSS;
			static constexpr mangle_t SECTOR_SIZE_ABS = (SECTOR_DEG * REDUCTION_DEN) / REDUCTION_NUM;

			static mangle_t to_mangle(angle_t x) { return static_cast<mangle_t>(x); }

			static mangle_t normalise(mangle_t angle) {
				angle %= PERIOD;
				if (angle < 0) angle += PERIOD;
				return angle;
			}

			static mangle_t short_diff(mangle_t a, mangle_t b) {
				mangle_t d = a - b;
				d %= PERIOD;
				if (d > HALF_PERIOD) d -= PERIOD;
				if (d < -HALF_PERIOD) d += PERIOD;
				return d;
			}

			static mangle_t abs(mangle_t x) { return (x < 0) ? -x : x; }

			static mangle_t to_actuator(mangle_t angle_abs) {
				int32_t tmp = static_cast<int32_t>(angle_abs) * REDUCTION_NUM;
				if (tmp >= 0)
					tmp = (tmp + REDUCTION_DEN/2) / REDUCTION_DEN;
				else
					tmp = (tmp - REDUCTION_DEN/2) / REDUCTION_DEN;
				return static_cast<mangle_t>(tmp);
			}

			static mangle_t to_abs_sensor(mangle_t angle_actuator) {
				int32_t tmp = static_cast<int32_t>(angle_actuator) * REDUCTION_DEN;
				if (tmp >= 0)
					tmp = (tmp + REDUCTION_NUM/2) / REDUCTION_NUM;
				else
					tmp = (tmp - REDUCTION_NUM/2) / REDUCTION_NUM;
				return static_cast<mangle_t>(tmp);
			}

			static mangle_t to_central_sector(mangle_t a) {
				if (a >= 0)
					return a / SECTOR_SIZE_ABS;
				else
					return (a - (SECTOR_SIZE_ABS - 1)) / SECTOR_SIZE_ABS;
			}

			static mangle_t to_electrical(mangle_t mech_angle, mangle_t P) {
				int32_t tmp = static_cast<int32_t>(mech_angle) * P;
				tmp %= PERIOD;
				if (tmp < 0) tmp += PERIOD;
				return static_cast<mangle_t>(tmp);
			}
		};

		// ============================================================================
		// Класс склейки показаний резольвера и абсолютного датчика
		// ============================================================================

		/*!
		 * \brief Реализует слияние абсолютного датчика, грубого и точного резольвера.
		 * \tparam Driver  один из драйверов (float_driver или int_driver)
		 *
		 * Данные измеренных[ углов передаются в конструкторе по ссылкам, которые должны
		 * существовать дольше, чем экземпляр данного класса. Результаты вычислений
		 * сохраняются в структуру present_s, переданную в конструктор.
		 */
		template<typename Driver>
		class resolver_absolute_fusion_t {
		public:
			using angle_t = typename Driver::angle_t;
			using mangle_t = typename Driver::mangle_t;
			#if ROBO_APP_BURST_VARTREE_ENABLED
			static constexpr auto angle_var = Driver::angle_var;
			static constexpr auto mangle_var = Driver::mangle_var;
			#endif
			/*! \brief Структура для хранения результатов и промежуточных данных. */
			struct present_s {
				bool fault_elec_out;   //!< флаг невязки между грубой и точной обмоткой
				bool fault_abs_out;    //!< флаг невязки после выбора сектора
				mangle_t D_raw;            //!< невязка электрических углов
				mangle_t D_abs;            //!< абсолютная невязка после выбора сектора
				mangle_t angle;            //!< результирующий угол привода
				bool enabled;
			};

			/*!
			 * \brief Конструктор.
			 * \param abs_raw   показания абсолютного датчика (на валу датчика)
			 * \param gross_raw показания грубой обмотки (электрический угол)
			 * \param fine_raw  показания точной обмотки (электрический угол)
			 * \param present   ссылка на структуру для сохранения результатов
			 */
			resolver_absolute_fusion_t(
				const angle_t& abs_raw,
				const angle_t& gross_raw,
				const angle_t& fine_raw,
				present_s& present
			) : abs_raw_(abs_raw)
			  , gross_raw_(gross_raw)
			  , fine_raw_(fine_raw)
			  , present_(present)
			{}

			#if ROBO_APP_BURST_VARTREE_ENABLED
			void regvar_present(robo::cstr _name) {
				var::types angle_var = Driver::angle_var;
				var::types mangle_var = Driver::mangle_var;
				var::push(_name);
				if (burst::var::actual_mode >= burst::var::mode::full) {
					var::push(RT("fault")); {
						var::reg(var::types::const_uint8, present_.fault_elec_out, RT("elec_out"));
						var::reg(var::types::const_uint8, present_.fault_abs_out, RT("abs_out"));
					}var::pop();
					var::push(RT("D")); {
						var::reg(mangle_var, present_.D_raw, RT("raw"));
						var::reg(mangle_var, present_.D_abs, RT("abs"));
					}var::pop();
					var::reg(mangle_var, present_.angle, RT("angle"));
					var::reg(var::types::const_uint8, present_.enabled, RT("enabled"));
				}var::pop();
			}
			#endif

			/*!
			 * \brief Выполняет вычисления и сохраняет результат в present_.angle.
			 * \return оценка угла привода (в масштабе драйвера)
			 */
			void run(void) {
				if (present_.enabled) {
					mangle_t a = Driver::to_mangle(abs_raw_);
					mangle_t g_elec = Driver::to_mangle(gross_raw_);
					mangle_t f_elec = Driver::to_mangle(fine_raw_);

					// Электрический критерий
					present_.D_raw = Driver::normalise(Driver::P_FINE * g_elec - Driver::P_GROSS * f_elec);
					mangle_t D_elec = Driver::short_diff(present_.D_raw, 0);
					present_.fault_elec_out = Driver::abs(D_elec) > Driver::D_THRESH;

					// Грубый механический угол на валу датчика
					mangle_t g_mech_abs = Driver::to_abs_sensor(g_elec / Driver::P_GROSS);

					// Центральный сектор по абсолютному датчику
					mangle_t central_sector = Driver::to_central_sector(a);

					// Три кандидата
					mangle_t candidates[3];
					for (int i = -1; i <= 1; ++i) {
						mangle_t sector = central_sector + i;
						candidates[i + 1] = sector * Driver::SECTOR_SIZE_ABS + g_mech_abs;
					}

					// Выбор ближайшего к a
					mangle_t best_candidate = candidates[0];
					mangle_t min_err = Driver::abs(a - candidates[0]);
					for (int i = 1; i < 3; ++i) {
						mangle_t err = Driver::abs(a - candidates[i]);
						if (err < min_err) {
							min_err = err;
							best_candidate = candidates[i];
						}
					}

					// Критерий D_abs
					present_.D_abs = best_candidate - a;
					present_.fault_abs_out = Driver::abs(present_.D_abs) > Driver::D_ABS_THRESH;

					// При ошибке возвращаем только абсолютный датчик
					if (present_.fault_elec_out || present_.fault_abs_out) {
						present_.angle = Driver::to_actuator(a);
					}

					// Уточнение по точной обмотке
					mangle_t coarse_act = Driver::to_actuator(best_candidate);
					mangle_t expected_fine_elec = Driver::to_electrical(coarse_act, Driver::P_FINE);
					mangle_t delta_elec = Driver::short_diff(f_elec, expected_fine_elec);
					mangle_t correction_mech = delta_elec / Driver::P_FINE;
					present_.angle = coarse_act + correction_mech;
				}
			}

		private:
			const angle_t& abs_raw_;    //!< ссылка на измеренный угол абсолютного датчика
			const angle_t& gross_raw_;  //!< ссылка на измеренный угол грубой обмотки
			const angle_t& fine_raw_;   //!< ссылка на измеренный угол точной обмотки
			present_s& present_;        //!< ссылка на структуру результатов
		};

	} // namespace resolver
} // namespace burst

#endif // burst_resolver_fusion_hpp