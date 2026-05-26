#ifndef NTCG163JF103_hpp
#define NTCG163JF103_hpp
#include "core/robosd_common.hpp"
namespace robo{
	namespace prf{
		namespace ntc{
			// Вспомогательные constexpr-функции (свободные)
			constexpr double log(double x) {
					if (x <= 0.0) return -1e100;
					double y = (x - 1.0) / (x + 1.0);
					double y2 = y * y;
					double term = y;
					double sum = term;
					for (int k = 1; k < 20; ++k) {
							term *= y2;
							sum += term / (2.0 * k + 1.0);
					}
					return 2.0 * sum;
			}
			template<typename T>
			constexpr T resistance(uint16_t adc, T r_fixed, T adc_max) {
					return r_fixed * (adc_max / T(adc) - T(1));
			}
			template<typename T>
			constexpr T temp_k(T resistance, T r0, T t0_k, T b) {
				double ln = log(static_cast<double>(resistance / r0));
				return T(1) / (T(1)/t0_k + (T(1)/b) * T(ln));
			}
			template<typename T>
			constexpr T temp_c(uint16_t adc, T r_fixed, T adc_max, T r0, T t0_k, T b) {
				T r = resistance(adc, r_fixed, adc_max);
				return temp_k(r, r0, t0_k, b) - T(273.15);
			}
			
			// Структура конфигурации для NTCG163JF103FT1
			template<typename T>
			struct NTCG163JF103FT1_t {
					static constexpr T ADC_MAX = T(4095);
					static constexpr T R_FIXED = T(10000);   // Нижний резистор делителя (Ом)
					static constexpr T R0      = T(10000);   // Сопротивление при 25°C (Ом)
					static constexpr T T0_K    = T(298.15);  // 25°C в Кельвинах
					static constexpr T B       = T(3435);    // B-константа (25/85)
			};
			
			// Основной класс таблицы
			template<typename T = float, size_t N = 100, typename Config = NTCG163JF103FT1_t<T>>
			struct table_t {
				T table[N];
				constexpr table_t() : table{} {
						const T step = Config::ADC_MAX / T(N - 1);
						for (size_t i = 0; i < N; ++i) {
								uint16_t adc = static_cast<uint16_t>(i * step);
								if (adc == 0) adc = 1;
								table[i] = temp_c(adc,
																			Config::R_FIXED,
																			Config::ADC_MAX,
																			Config::R0,
																			Config::T0_K,
																			Config::B);
						}
				}

				constexpr T get(uint16_t adc) const {
						if (adc == 0) adc = 1;
						const T step = Config::ADC_MAX / T(N - 1);
						T idx = static_cast<T>(adc) / step;
						size_t i0 = static_cast<size_t>(idx);
						if (i0 >= N - 1) return table[N - 1];
						T frac = idx - static_cast<T>(i0);
						return table[i0] + frac * (table[i0 + 1] - table[i0]);
				}
			};
		}
	}
}
#endif