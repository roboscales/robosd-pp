#ifndef burst_angle_core_hpp
#define burst_angle_core_hpp

#include "burst++/modules/actor.hpp"
#if ROBO_APP_ULTRACOMPACT == 0
#include "burst++/burst.hpp"
#else
#include "burst++/burst_common.hpp"
#endif
#include "burst++/vartree.hpp"

namespace burst {

    //==============================================================
    // Политики преобразования ceiled -> angle
    //   + difference(a,b) – циклическая разность в диапазоне (-pi, pi]
    //==============================================================

    struct angle_policy_fixed16_to_int16 {
        using ceiled_t = uint16_t;
        using angle_t = int16_t;

        static angle_t convert(ceiled_t ceiled, uint8_t resolution) {
            int16_t v = static_cast<int16_t>(ceiled);
            if (resolution < 16) {
                int shift = 16 - resolution;
                v <<= shift;
            }
            return v;
        }

        static angle_t difference(angle_t current, angle_t previous) {
            int32_t diff = static_cast<int32_t>(current) - static_cast<int32_t>(previous);
            constexpr int32_t range = 65536;
            constexpr int32_t half = 32767;
            if (diff > half)  diff -= range;
            if (diff < -half) diff += range;
            return static_cast<int16_t>(diff);
        }

#if ROBO_APP_BURST_VARTREE_ENABLED
        static void reg_angle(robo::cstr name, const angle_t& val) {
            burst::var::reg(burst::var::types::const_int16, val, name);
        }
#endif
    };

    struct angle_policy_fixed32_to_int32 {
        using ceiled_t = uint32_t;
        using angle_t = int32_t;

        static angle_t convert(ceiled_t ceiled, uint8_t resolution) {
            int32_t v = static_cast<int32_t>(ceiled);
            if (resolution < 32) {
                int shift = 32 - resolution;
                v <<= shift;
            }
            return v;
        }

        static angle_t difference(angle_t current, angle_t previous) {
            int64_t diff = static_cast<int64_t>(current) - static_cast<int64_t>(previous);
            constexpr int64_t range = 4294967296LL;
            constexpr int64_t half = 2147483647LL;
            if (diff > half)  diff -= range;
            if (diff < -half) diff += range;
            return static_cast<int32_t>(diff);
        }

#if ROBO_APP_BURST_VARTREE_ENABLED
        static void reg_angle(robo::cstr name, const angle_t& val) {
            burst::var::reg(burst::var::types::const_int32, val, name);
        }
#endif
    };

    struct angle_policy_float_from_uint16 {
        using ceiled_t = uint16_t;
        using angle_t = float;

        static angle_t convert(ceiled_t ceiled, uint8_t /*resolution*/) {
            constexpr float inv_range = 1.0f / 65536.0f;
            constexpr float two_pi = 2.0f * robo::pi<float>;
            constexpr float pi = robo::pi<float>;
            return static_cast<float>(ceiled) * inv_range * two_pi - pi;
        }

        static angle_t difference(angle_t current, angle_t previous) {
            float diff = current - previous;
            constexpr float two_pi = 2.0f * robo::pi<float>;
            while (diff > robo::pi<float>) diff -= two_pi;
            while (diff <= -robo::pi<float>) diff += two_pi;
            return diff;
        }

#if ROBO_APP_BURST_VARTREE_ENABLED
        static void reg_angle(robo::cstr name, const angle_t& val) {
            burst::var::reg(burst::var::types::const_real, val, name);
        }
#endif
    };

    struct angle_policy_double_from_uint16 {
        using ceiled_t = uint16_t;
        using angle_t = double;

        static angle_t convert(ceiled_t ceiled, uint8_t /*resolution*/) {
            constexpr double inv_range = 1.0 / 65536.0;
            constexpr double two_pi = 2.0 * robo::pi<double>;
            constexpr double pi = robo::pi<double>;
            return static_cast<double>(ceiled) * inv_range * two_pi - pi;
        }

        static angle_t difference(angle_t current, angle_t previous) {
            double diff = current - previous;
            constexpr double two_pi = 2.0 * robo::pi<double>;
            while (diff > robo::pi<double>) diff -= two_pi;
            while (diff <= -robo::pi<double>) diff += two_pi;
            return diff;
        }

#if ROBO_APP_BURST_VARTREE_ENABLED
        static void reg_angle(robo::cstr name, const angle_t& val) {
            burst::var::reg(burst::var::types::const_ext, val, name);
        }
#endif
    };

    struct angle_policy_float_from_uint32 {
        using ceiled_t = uint32_t;
        using angle_t = float;

        static angle_t convert(ceiled_t ceiled, uint8_t /*resolution*/) {
            constexpr float inv_range = 1.0f / 4294967296.0f;
            constexpr float two_pi = 2.0f * robo::pi<float>;
            constexpr float pi = robo::pi<float>;
            return static_cast<float>(ceiled) * inv_range * two_pi - pi;
        }

        static angle_t difference(angle_t current, angle_t previous) {
            float diff = current - previous;
            constexpr float two_pi = 2.0f * robo::pi<float>;
            while (diff > robo::pi<float>) diff -= two_pi;
            while (diff <= -robo::pi<float>) diff += two_pi;
            return diff;
        }

#if ROBO_APP_BURST_VARTREE_ENABLED
        static void reg_angle(robo::cstr name, const angle_t& val) {
            burst::var::reg(burst::var::types::const_real, val, name);
        }
#endif
    };

    struct angle_policy_double_from_uint32 {
        using ceiled_t = uint32_t;
        using angle_t = double;

        static angle_t convert(ceiled_t ceiled, uint8_t /*resolution*/) {
            constexpr double inv_range = 1.0 / 4294967296.0;
            constexpr double two_pi = 2.0 * robo::pi<double>;
            constexpr double pi = robo::pi<double>;
            return static_cast<double>(ceiled) * inv_range * two_pi - pi;
        }

        static angle_t difference(angle_t current, angle_t previous) {
            double diff = current - previous;
            constexpr double two_pi = 2.0 * robo::pi<double>;
            while (diff > robo::pi<double>) diff -= two_pi;
            while (diff <= -robo::pi<double>) diff += two_pi;
            return diff;
        }

#if ROBO_APP_BURST_VARTREE_ENABLED
        static void reg_angle(robo::cstr name, const angle_t& val) {
            burst::var::reg(burst::var::types::const_ext, val, name);
        }
#endif
    };

    //==============================================================
    // Автовыбор политики
    //==============================================================
    template<typename Ceiled, typename Angle>
    struct angle_policy;

    template<> struct angle_policy<uint16_t, int16_t> : angle_policy_fixed16_to_int16 {};
    template<> struct angle_policy<uint16_t, float> : angle_policy_float_from_uint16 {};
    template<> struct angle_policy<uint16_t, double> : angle_policy_double_from_uint16 {};

    template<> struct angle_policy<uint32_t, int32_t> : angle_policy_fixed32_to_int32 {};
    template<> struct angle_policy<uint32_t, float> : angle_policy_float_from_uint32 {};
    template<> struct angle_policy<uint32_t, double> : angle_policy_double_from_uint32 {};

    //==============================================================
    // Адаптер дискретного датчика угла
    //==============================================================
    template<class number, class driver,
        typename Ceiled = uint32_t,
        typename Angle = typename number::signal_t,
        typename Policy = angle_policy<Ceiled, Angle>>
        class angle_adapter_t : public actor {
        public:
            using raw_t = typename driver::raw_t;
            using ceiled_t = typename Policy::ceiled_t;
            using angle_t = typename Policy::angle_t;

            struct config_s {
                actor::config_s tag;
                uint8_t  raw_resolution;
                uint8_t  output_resolution;
                ceiled_t offset;
            };

#define ANGLE_ADAPTER_CONFIG(a) ANGLE_ADAPTER_CONFIG_(a)
#define ANGLE_ADAPTER_CONFIG_(a) \
    { \
        ACTOR_CONFIG(a) \
        , a##_RAW_RESOLUTION \
        , a##_OUTPUT_RESOLUTION \
        , a##_OFFSET \
    }

            struct present_s {
                actor::present_s tag;
                raw_t    raw;
                ceiled_t ceiled;
                uint8_t  driver_success;
                uint32_t error_counter;
                angle_t  angle;
            };

#if ROBO_APP_BURST_VARTREE_ENABLED
            virtual void do_regvar_present(void) override {
                using namespace burst::var;
                ACTOR_PRESENT_S(p);
                push(RT("raw"));
                reg(descriptor_enco(sizeof(raw_t), false, true, false), p.raw, RT("value"));
                pop();
                if constexpr (sizeof(ceiled_t) == 2)
                    reg(types::const_uint16, p.ceiled, RT("ceiled"));
                else
                    reg(types::const_uint32, p.ceiled, RT("ceiled"));
                reg(types::const_uint8, p.driver_success, RT("success"));
                reg(types::uint32, p.error_counter, RT("errcnt"));
                Policy::reg_angle(RT("angle"), p.angle);
            }

            virtual void do_regvar_conf(void) override {
                using namespace burst::var;
                if (actual_mode >= mode::tuning) {
                    ACTOR_CONFIG_S(c);
                    reg(types::uint8, c.raw_resolution, RT("raw_bits"), true);
                    reg(types::uint8, c.output_resolution, RT("out_bits"), true);
                    if constexpr (sizeof(ceiled_t) == 2)
                        reg(types::const_uint16, c.offset, RT("offset"), true);
                    else
                        reg(types::const_uint32, c.offset, RT("offset"), true);
                }
            }
#endif

        public:
#if ROBO_APP_ULTRACOMPACT == 0
            angle_adapter_t(const config_s& _config, present_s& _present)
                : actor(_config.tag, _present.tag) {}
            angle_adapter_t(const config_s& _config, present_s& _present, subsystem& _sub)
                : actor(_config.tag, _present.tag, _sub) {}
#else
            angle_adapter_t(const config_s& _config, present_s& _present)
                : actor(_config.tag, _present.tag) {}
#endif

            virtual void begin(void) override {
                ACTOR_CONFIG_S(c);
                ACTOR_PRESENT_S(p);
                p = {};
                shift_raw_ = c.output_resolution - c.raw_resolution;

                if constexpr (std::is_integral<angle_t>::value) {
                    constexpr unsigned max_bits = sizeof(ceiled_t) * 8;
                    ROBO_APP_ASSERT(c.output_resolution <= max_bits);
                }
            }

            virtual void run(void) override {
                ACTOR_CONFIG_S(c);
                ACTOR_PRESENT_S(p);

                bool ok = driver::query(p.raw);
                p.driver_success = ok ? 1 : 0;
                if (ok) {
                    ceiled_t shifted = static_cast<ceiled_t>(p.raw) << shift_raw_;
                    p.ceiled = shifted + c.offset;
                    p.angle = Policy::convert(p.ceiled, c.output_resolution);
                }
                else {
                    ++p.error_counter;
                }
            }

        private:
            uint8_t shift_raw_ = 0;
    };

    //==============================================================
    // Дифференциатор угла (приращение за такт)
    //==============================================================
    template<class Policy>
    class angle_differentiator_t : public actor {
    public:
        using angle_t = typename Policy::angle_t;

        struct config_s {
            actor::config_s tag;
        };

#define ANGLE_DIFF_CONFIG(a) ANGLE_DIFF_CONFIG_(a)
#define ANGLE_DIFF_CONFIG_(a) \
    { \
        ACTOR_CONFIG(a) \
    }

        struct present_s {
            actor::present_s tag;
            angle_t  increment;         // разность между текущим и предыдущим углом
            angle_t  previous_angle;
            bool     first_run;
        };

        angle_differentiator_t(const config_s& _config, present_s& _present, angle_t& _angle_ref)
            : actor(_config.tag, _present.tag), angle_ref_(_angle_ref) {}

        virtual void begin(void) override {
            ACTOR_PRESENT_S(p);
            p.first_run = true;
            p.increment = angle_t{ 0 };
            p.previous_angle = angle_t{ 0 };
        }

        virtual void run(void) override {
            ACTOR_PRESENT_S(p);
            if (p.first_run) {
                p.previous_angle = angle_ref_;
                p.increment = angle_t{ 0 };
                p.first_run = false;
            }
            else {
                p.increment = Policy::difference(angle_ref_, p.previous_angle);
                p.previous_angle = angle_ref_;
            }
        }

#if ROBO_APP_BURST_VARTREE_ENABLED
        virtual void do_regvar_present(void) override {
            using namespace burst::var;
            ACTOR_PRESENT_S(p);
            Policy::reg_angle(RT("increment"), p.increment);
            reg(types::const_uint8, p.first_run, RT("first_run"));
        }

        virtual void do_regvar_conf(void) override { /* пусто */ }
#endif

    private:
        angle_t& angle_ref_;
    };

    //==============================================================
    // Аккумулятор угла
    //   - init_count тактов использует абсолютный угол
    //   - затем суммирует приращения
    //   - delta_acc накапливается и сбрасывается извне
    //==============================================================
    template<class Policy>
    class angle_accumulator_t : public actor {
    public:
        using angle_t = typename Policy::angle_t;

        struct config_s {
            actor::config_s tag;
            uint32_t init_count;     // количество тактов инициализации абсолютным значением
            bool     inverse;        // true – инвертировать направление приращений
            angle_t  offset;         // смещение нуля (добавляется к абсолютному углу при инициализации)
        };

#define ANGLE_ACCUM_CONFIG(a) ANGLE_ACCUM_CONFIG_(a)
#define ANGLE_ACCUM_CONFIG_(a) \
    { \
        ACTOR_CONFIG(a) \
        , a##_INIT_COUNT \
        , a##_INVERSE \
        , a##_OFFSET \
    }

        struct present_s {
            actor::present_s tag;
            angle_t  position;       // результирующее положение
            angle_t  delta_acc;      // накопленное приращение (сбрасывается внешним кодом)
            int32_t  init_counter;   // оставшиеся такты инициализации
            bool     ready;          // true – рабочий режим (суммирование)
        };

        // Принимает ссылки на абсолютный угол и на приращение
        angle_accumulator_t(const config_s& _config, present_s& _present,
            angle_t& _abs_angle, angle_t& _increment)
            : actor(_config.tag, _present.tag),
            abs_angle_(_abs_angle),
            increment_(_increment) {}

        virtual void begin(void) override {
            ACTOR_CONFIG_S(c);
            ACTOR_PRESENT_S(p);
            p.init_counter = c.init_count;
            p.ready = false;
            p.position = c.offset;
            p.delta_acc = angle_t{ 0 };
        }

        virtual void run(void) override {
            ACTOR_CONFIG_S(c);
            ACTOR_PRESENT_S(p);
            if (!p.ready) {
                // Инициализация: берём абсолютный угол, инвертируем при необходимости
                angle_t abs = abs_angle_;
                if (c.inverse) {
                    // -abs, безопасный для циклического представления
                    p.position = Policy::difference(angle_t{ 0 }, abs) + c.offset;
                }
                else {
                    p.position = abs + c.offset;
                }
                if (--p.init_counter <= 0) {
                    p.ready = true;
                }
            }
            else {
                // Рабочий режим: добавляем приращение
                angle_t inc = increment_;
                if (c.inverse) {
                    inc = Policy::difference(angle_t{ 0 }, inc);
                }
                p.position += inc;          // циклическое переполнение допустимо для целых
                p.delta_acc += inc;         // внешний код может обнулить delta_acc
            }
        }

#if ROBO_APP_BURST_VARTREE_ENABLED
        virtual void do_regvar_present(void) override {
            using namespace burst::var;
            ACTOR_PRESENT_S(p);
            Policy::reg_angle(RT("position"), p.position);
            Policy::reg_angle(RT("delta_acc"), p.delta_acc);
            reg(types::const_int32, p.init_counter, RT("init_cnt"));
            reg(types::const_uint8, p.ready, RT("ready"));
        }

        virtual void do_regvar_conf(void) override {
            using namespace burst::var;
            if (actual_mode >= mode::tuning) {
                ACTOR_CONFIG_S(c);
                reg(types::uint32, c.init_count, RT("init_count"), true);
                reg(types::uint8, c.inverse, RT("inverse"), true);
                Policy::reg_angle(RT("offset"), c.offset);
            }
        }
#endif

    private:
        angle_t& abs_angle_;
        angle_t& increment_;
    };

} // namespace burst

#endif // burst_angle_core_hpp