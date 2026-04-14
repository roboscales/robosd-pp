#include "resolver_fusion.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// Параметры приводов (примеры)
// ============================================================================

struct my_float_params {
    static constexpr double P_GROSS = 3.0;
    static constexpr double P_FINE = 64.0;
    static constexpr double MAX_ACTUATOR_ANGLE = 720.0;
    static constexpr double REDUCTION = 720.0 / 355.0;
    static constexpr double D_THRESH = 100.0;
    static constexpr double D_ABS_THRESH = 15.0;
};

struct my_int_params {
    static constexpr int32_t P_GROSS = 3;
    static constexpr int32_t P_FINE = 64;
    static constexpr int32_t MAX_ACTUATOR_ANGLE = 720000;
    static constexpr int32_t REDUCTION_NUM = 144;
    static constexpr int32_t REDUCTION_DEN = 71;
    static constexpr int32_t D_THRESH = 100000;
    static constexpr int32_t D_ABS_THRESH = 15000;
};

struct my_frac_params {
    static constexpr int32_t P_GROSS = 3;
    static constexpr int32_t P_FINE = 64;
    static constexpr int32_t MAX_ACTUATOR_ANGLE = 131072;
    static constexpr int32_t REDUCTION_NUM = 144;
    static constexpr int32_t REDUCTION_DEN = 71;
    static constexpr int32_t D_THRESH = 18204;
    static constexpr int32_t D_ABS_THRESH = 2730;
};

// ============================================================================
// Функция тестирования
// ============================================================================

template<typename DriverType, typename Scale>
void test_driver(const char* name, Scale, bool add_noise) {
    using Fusion = burst::resolver::resolver_absolute_fusion<DriverType>;
    using angle_t = typename DriverType::angle_t;
    using mangle_t = typename DriverType::mangle_t;
    using present_s = typename Fusion::present_s;

    const double NOISE_ABS_DEG = add_noise ? 30.0 : 0.0;
    const double NOISE_GROSS_ELEC_DEG = add_noise ? 3.0 : 0.0;
    const double NOISE_FINE_ELEC_DEG = add_noise ? 3.0 : 0.0;

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    const double MAX_ANGLE_DEG = 720.0;
    const int N_POINTS = 2000;
    std::vector<double> true_angles(N_POINTS);
    for (int i = 0; i < N_POINTS; ++i)
        true_angles[i] = MAX_ANGLE_DEG * i / (N_POINTS - 1);

    double reduction;
    if constexpr (requires { DriverType::REDUCTION; })
        reduction = DriverType::REDUCTION;
    else
        reduction = static_cast<double>(DriverType::REDUCTION_NUM) / DriverType::REDUCTION_DEN;

    const double P_GROSS = DriverType::P_GROSS;
    const double P_FINE = DriverType::P_FINE;

    std::vector<double> errors_non_fault;
    int fault_elec_cnt = 0, fault_abs_cnt = 0, fault_any_cnt = 0;

    for (double true_deg : true_angles) {
        double abs_true_deg = std::fmod(true_deg / reduction, 360.0);
        double gross_true_deg = std::fmod(P_GROSS * true_deg, 360.0);
        double fine_true_deg = std::fmod(P_FINE * true_deg, 360.0);

        double abs_noisy = abs_true_deg;
        double gross_noisy = gross_true_deg;
        double fine_noisy = fine_true_deg;
        if (add_noise) {
            abs_noisy += dist(rng) * NOISE_ABS_DEG;
            gross_noisy += dist(rng) * NOISE_GROSS_ELEC_DEG;
            fine_noisy += dist(rng) * NOISE_FINE_ELEC_DEG;
        }

        auto norm = [](double x) {
            x = std::fmod(x, 360.0);
            if (x < 0) x += 360.0;
            return x;
        };
        abs_noisy = norm(abs_noisy);
        gross_noisy = norm(gross_noisy);
        fine_noisy = norm(fine_noisy);

        angle_t abs_val, gross_val, fine_val;
        if constexpr (std::is_same_v<Scale, burst::resolver::degree_scale>) {
            abs_val = static_cast<angle_t>(abs_noisy);
            gross_val = static_cast<angle_t>(gross_noisy);
            fine_val = static_cast<angle_t>(fine_noisy);
        } else if constexpr (std::is_same_v<Scale, burst::resolver::milli_degree_scale>) {
            abs_val = static_cast<angle_t>(std::round(abs_noisy * 1000.0));
            gross_val = static_cast<angle_t>(std::round(gross_noisy * 1000.0));
            fine_val = static_cast<angle_t>(std::round(fine_noisy * 1000.0));
        } else if constexpr (std::is_same_v<Scale, burst::resolver::frac_pi_scale>) {
            constexpr double FACTOR = 65536.0 / 360.0;
            abs_val = static_cast<angle_t>(std::round(abs_noisy * FACTOR));
            gross_val = static_cast<angle_t>(std::round(gross_noisy * FACTOR));
            fine_val = static_cast<angle_t>(std::round(fine_noisy * FACTOR));
        }

        present_s pres;
        Fusion fusion(abs_val, gross_val, fine_val, pres);
        mangle_t result = fusion.run();

        if (pres.fault_elec_out) fault_elec_cnt++;
        if (pres.fault_abs_out) fault_abs_cnt++;
        if (pres.fault_elec_out || pres.fault_abs_out) fault_any_cnt++;

        double result_deg;
        if constexpr (std::is_same_v<Scale, burst::resolver::degree_scale>)
            result_deg = static_cast<double>(result);
        else if constexpr (std::is_same_v<Scale, burst::resolver::milli_degree_scale>)
            result_deg = static_cast<double>(result) / 1000.0;
        else if constexpr (std::is_same_v<Scale, burst::resolver::frac_pi_scale>)
            result_deg = static_cast<double>(result) * 360.0 / 65536.0;

        double err = result_deg - true_deg;
        err = std::fmod(err + 180.0, 360.0);
        if (err < 0) err += 360.0;
        err -= 180.0;

        if (!(pres.fault_elec_out || pres.fault_abs_out))
            errors_non_fault.push_back(err);
    }

    double max_err = 0.0, sum_abs = 0.0, sum_sq = 0.0;
    for (double e : errors_non_fault) {
        max_err = std::max(max_err, std::fabs(e));
        sum_abs += std::fabs(e);
        sum_sq += e * e;
    }
    size_t n_non_fault = errors_non_fault.size();
    double mean_abs = n_non_fault ? sum_abs / n_non_fault : 0.0;
    double rmse = n_non_fault ? std::sqrt(sum_sq / n_non_fault) : 0.0;

    std::cout << "=== " << name << (add_noise ? " (с шумом)" : " (без шума)") << " ===\n";
    std::cout << "Неаварийных точек: " << n_non_fault << " из " << true_angles.size() << "\n";
    std::cout << "Аварий по D_elec: " << fault_elec_cnt << "\n";
    std::cout << "Аварий по D_abs: " << fault_abs_cnt << "\n";
    std::cout << "Всего аварий: " << fault_any_cnt << " (" << 100.0 * fault_any_cnt / true_angles.size() << "%)\n";
    std::cout << "Макс. ошибка (неавар.): " << max_err << " град\n";
    std::cout << "Ср. абс. ошибка: " << mean_abs << " град\n";
    std::cout << "СКО (RMSE): " << rmse << " град\n\n";
}

// ============================================================================
// Главная функция
// ============================================================================

int main() {
    using namespace burst::resolver;

    // Без шума (проверка корректности)
    test_driver<float_driver<degree_scale, my_float_params>, degree_scale>(
        "float_driver (градусы)", degree_scale{}, false);
    test_driver<int_driver<milli_degree_scale, my_int_params>, milli_degree_scale>(
        "int_driver (миллиградусы)", milli_degree_scale{}, false);
    test_driver<int_driver<frac_pi_scale, my_frac_params>, frac_pi_scale>(
        "int_driver (отсчёты 65536)", frac_pi_scale{}, false);

    // С шумом
    test_driver<float_driver<degree_scale, my_float_params>, degree_scale>(
        "float_driver (градусы)", degree_scale{}, true);
    test_driver<int_driver<milli_degree_scale, my_int_params>, milli_degree_scale>(
        "int_driver (миллиградусы)", milli_degree_scale{}, true);
    test_driver<int_driver<frac_pi_scale, my_frac_params>, frac_pi_scale>(
        "int_driver (отсчёты 65536)", frac_pi_scale{}, true);

    return 0;
}