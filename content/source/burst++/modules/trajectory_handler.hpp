#ifndef __TRAJECTORY_HPP__
#define __TRAJECTORY_HPP__
#include "core/robosd_list.hpp"
#include "core/robosd_ring_buf.hpp"
#include "burst++/math.hpp"

namespace burst
{

    template<class number> struct TrajectoryInit
    {
        using parameter_t = typename number::parameter_t;
        using signal_t = typename number::signal_t;
        using long_signal_t = typename number::long_signal_t;
        using usignal_t = typename number::usignal_t;
        using ulong_signal_t = typename number::ulong_signal_t;

        long_signal_t start = 0;
        long_signal_t end = 0;
        long_signal_t speed = 0;
        long_signal_t deceleration = 0;
        long_signal_t acceleration = 0;

        // Масштаб времени: количество микросекунд в одной секунде (например, 1000000).
        // Ускорение/замедление задаются в обычных единицах (скорость/сек),
        // а внутри время пересчитывается в микросекунды.
        long_signal_t scale = 0;

        parameter_t profile_type = 0;
    };

    template<class Init, class number> struct TrajectoryPoint
    {
        using parameter_t = typename number::parameter_t;
        using signal_t = typename number::signal_t;
        using long_signal_t = typename number::long_signal_t;
        using usignal_t = typename number::usignal_t;
        using ulong_signal_t = typename number::ulong_signal_t;
        
        TrajectoryInit<number> * params;  

        long_signal_t start = 0;
        long_signal_t end = 0;

        ulong_signal_t t = 0;
        ulong_signal_t time_acc = 0;
        ulong_signal_t time_dec = 0;
        ulong_signal_t time_cruise = 0;
        ulong_signal_t time_overall= 0;
        ulong_signal_t time_start = 0;
        ulong_signal_t *time_current_global;

        long_signal_t out= 0;

    	long_signal_t l = 0;
    	long_signal_t l_acc = 0;
    	long_signal_t l_dec = 0;

        // Признак завершения траектории точки (достигнута целевая скорость).
        bool done = false;
    };

    template<class number, class TrajectoryInit> struct TrajectoryProfile
    {
        using parameter_t = typename number::parameter_t;
        using signal_t = typename number::signal_t;
        using long_signal_t = typename number::long_signal_t;
        using usignal_t = typename number::usignal_t;
        using ulong_signal_t = typename number::ulong_signal_t;

        // Словарь профилей: по номеру профиля (profile_index) возвращается наследник TrajectoryProfile.
        using profileList_t = robo::list::unique<TrajectoryProfile*, parameter_t>;
        using profilePair_t = robo::list::pair<TrajectoryProfile*, parameter_t, profileList_t, true>;

        parameter_t profile_index = -1;

        // Ссылка на самого себя для хранения в словаре (словарь хранит указатели на полиморфные профили).
        TrajectoryProfile* self = this;
        profilePair_t profile_pair;

        // Словарь, в который профиль регистрируется при создании.
        profileList_t* dict_ = nullptr;

        TrajectoryProfile(profileList_t& dict) : profile_pair(self, profile_index), dict_(&dict)
        {
        }

        // Регистрация профиля в словаре по его profile_index.
        void registerProfile()
        {
            profile_pair.set_key(profile_index);
            profile_pair.attach_to(*dict_);
        }

        virtual ~TrajectoryProfile() = default;

        virtual void Approximer(TrajectoryPoint<TrajectoryInit, number>& point_, TrajectoryInit& init_)
        {
        }
        virtual void Interpolator(TrajectoryPoint<TrajectoryInit, number>& point_, ulong_signal_t dt)
        {
        }
    };

    // Реализация TrajectoryProfile для линейной интерполяции скорости.
    template<class number, class TrajectoryInit> class LinearSpeedProfile : public TrajectoryProfile<number, TrajectoryInit>
    {
    public:
        using parameter_t = typename number::parameter_t;
        using signal_t = typename number::signal_t;
        using long_signal_t = typename number::long_signal_t;
        using usignal_t = typename number::usignal_t;
        using ulong_signal_t = typename number::ulong_signal_t;

        using super = TrajectoryProfile<number, TrajectoryInit>;

        LinearSpeedProfile(typename super::profileList_t& dict) : super(dict)
        {
            this->profile_index = 1;
            this->registerProfile();
        }

        long_signal_t fromParrotsTomDegS(long_signal_t parrots)
        {

			float z = 100.0f;
			float p = 15.0f;
			float t = 40.0f * 20 * (1.0f/21000.0f);
			float spPresc = 5.0f;
			return (long_signal_t)(1000000 * ((parrots)/(z * p * 6.0f * t * pow(2.0f, (spPresc)) *360)));
        }

        long_signal_t frommDegSToParrots(long_signal_t mDeg)
        {
			float z = 100.0f;
			float p = 15.0f;
			float t = 40.0f * 20 * (1.0f/21000.0f);
			float spPresc = 5.0f;
			return (long_signal_t)((mDeg * z * p * 6.0f * t *  pow(2.0f, (spPresc)) * 360) / 1000000);
        }

        void Approximer(TrajectoryPoint<TrajectoryInit, number>& point_, TrajectoryInit& init_) override
        {
            // Расчет времени разгона/торможения на основе исходных данных.
            // start и end - сигналы скорости, acceleration/deceleration - ускорение/замедление.
            long_signal_t start = (init_.start);
            long_signal_t end = (init_.end);
            long_signal_t acceleration = (init_.acceleration);
            long_signal_t deceleration = (init_.deceleration);
            // Масштаб: микросекунд в секунде (ускорение задано в скорость/сек).
            long_signal_t scale = init_.scale;

            // Сохраняем начальную и конечную скорость в точке для интерполятора.
            point_.start = start;
            point_.end = end;

            // Сброс времени.
            point_.time_acc = 0;
            point_.time_dec = 0;
            point_.t = 0;

            long_signal_t delta = end - start;

            // Время разгона/торможения пересчитываем в микросекунды:
            // t[сек] = delta / acceleration  =>  t[мкс] = delta * scale / acceleration.
            // Умножаем до деления, чтобы избежать потери точности при целочисленном делении.
            if (delta > 0)
            {
                point_.time_acc = (acceleration != 0) ? (delta * scale / acceleration) : 0;
            }
            else if (delta < 0)
            {
                point_.time_dec = (deceleration != 0) ? (-delta * scale / deceleration) : 0;
            }

            point_.time_overall = point_.time_acc + point_.time_dec;
        }

        void Interpolator(TrajectoryPoint<TrajectoryInit, number>& point_, ulong_signal_t dt) override
        {
            long_signal_t start = point_.start;
            long_signal_t end = point_.end;

            // Накапливаем текущее время траектории.
            point_.t = dt - point_.time_start;

            if (point_.time_overall <= 0)
            {
                // Время не определено - скорость не меняется.
                point_.out = start;
                point_.done = true;
            }
            else if (point_.t >= point_.time_overall)
            {
            	point_.t = point_.time_overall;
                // Траектория завершена - выходим на конечную скорость.
                point_.out = end;
                point_.done = true;
            }
            else
            {
                // Расчет скорости в момент времени по формуле линейной интерполяции.
                // dt - приращение времени, задаваемое разработчиком.

                long_signal_t delta_v = end - start;
                point_.out = start + (long_signal_t)(delta_v * ((float)point_.t / (float)point_.time_overall));

            }

        }
    };

    // =============================================================================
    // НОВЫЙ ПРОФИЛЬ: LinearPositionProfile (Упрощенный, только по скорости)
    // =============================================================================

    template<class number, class TrajectoryInit> class LinearPositionProfile : public TrajectoryProfile<number, TrajectoryInit>
    {
    public:
        using parameter_t = typename number::parameter_t;
        using signal_t = typename number::signal_t;
        using long_signal_t = typename number::long_signal_t;
        using usignal_t = typename number::usignal_t;
        using ulong_signal_t = typename number::ulong_signal_t;
        using super = TrajectoryProfile<number, TrajectoryInit>;

        LinearPositionProfile(typename super::profileList_t& dict) : super(dict)
        {
            this->profile_index = 11; // Сохраняем старый индекс для совместимости
            this->registerProfile();
        }

        // Вспомогательная функция для целочисленного абсолютного значения (если std::abs не работает с long_signal_t)
        static long_signal_t abs_long(long_signal_t val) {
            return (val < 0) ? -val : val;
        }

        void Approximer(TrajectoryPoint<TrajectoryInit, number>& point_, TrajectoryInit& init_) override
        {
            // --- Упрощенный расчет: Время определяется только по скорости и расстоянию ---

            long_signal_t start = init_.start;
            long_signal_t end = init_.end;
            long_signal_t speed = init_.speed;

            point_.start = start;
            point_.end = end;

            // Сброс времени.
            point_.time_acc = 0;
            point_.time_dec = 0;
            point_.t = 0;

            long_signal_t delta = end - start;

            if (speed == 0) {
                point_.time_overall = 0;
                return;
            }

            // Расстояние L, время T = L / V.
            // Поскольку мы игнорируем A и D, считаем весь путь крейсерским.
            // Время в микросекундах: T[мкс] = |End - Start| * Scale / Speed
            point_.time_overall = (abs_long(delta) * init_.scale) / speed;

        }

        void Interpolator(TrajectoryPoint<TrajectoryInit, number>& point_, ulong_signal_t dt) override
        {
            // --- Интерполяция: Чистая линейная интерполяция по времени ---

            // Накапливаем текущее время траектории.
            point_.t = dt - point_.time_start;

            if (point_.time_overall <= 0 || point_.start == point_.end)
            {
                point_.out = point_.start;
                point_.done = true;
                return;
            }


            if (point_.t >= point_.time_overall)
            {
                point_.out = point_.end;
                point_.done = true;
                return;
            }

            // Расчет прогресса: t / T_total. Используем float для точности расчета пропорции,
            // но результат округляем обратно в long_signal_t.
            float progress_ratio = (float)point_.t / (float)point_.time_overall;

            // Линейная интерполяция: P(t) = Start + (End - Start) * ratio
            long_signal_t delta_v = point_.end - point_.start;

            // Используем float для промежуточного расчета, так как это единственный способ сохранить точность
            // при работе с пропорциями времени.
            float pos_offset_float = (float)point_.start + (float)(delta_v * progress_ratio);

            // Сохраняем результат в long_signal_t
            point_.out = (long_signal_t)round(pos_offset_float);
        }
    };


    template<class Init, class Point, class number> class TrajectoryHandler : public actor
    {
    public:
        using parameter_t = typename number::parameter_t;
        using signal_t = typename number::signal_t;
        using long_signal_t = typename number::long_signal_t;
        using usignal_t = typename number::usignal_t;
        using ulong_signal_t = typename number::ulong_signal_t;

        using profile_t = TrajectoryProfile<number, Init>;
        using profileList_t = typename profile_t::profileList_t;

        using super = actor;

        // Состояния траектории.
        enum class Trajectory_States
        {
            AWAITING,   // Ожидание точек
            HANDLING,   // Отрабатывается
        };

        // Очередь точек: store - пул свободных точек, actual - очередь точек на обработку.
        static constexpr uint8_t bits = 4;
        static constexpr size_t count = 1 << bits;
        using TrajectoryQueue = robo::ring_t<bits, Point*>;

        TrajectoryQueue actual, store;

        // Словарь профилей траектории (часть TrajectoryHandler).
        profileList_t profileDict;

		#pragma pack(push, 4)
        struct present_s
        {
        	super::present_s super_present;
        	Point* HandlingPoint = nullptr;
        	Point ObserverPoint;
        	Trajectory_States State;
        };

		#pragma pack(push, 4)
        struct config_s
        {
        	super::config_s super_config;
        };


        TrajectoryHandler(config_s& _config, present_s& _present) : super(_config.super_config, _present.super_present)
        {
            // конструктор объекта 
        }

        void SetState(Trajectory_States newState)
        {
        	ACTOR_PRESENT_S(p);
        	p.State = newState;
        }

        // Инициализация пула свободных точек.
        virtual void begin(void)
        {
        	ACTOR_PRESENT_S(p);
            static Point points[count];
            for (size_t i = 0; i < count; ++i)
            {
                store.put(&points[i]);
            }
            p.State = Trajectory_States::AWAITING;
        }

        bool checkForNewPoints()
        {
            return actual.count() > 0;
        }

    private:
        // Взять точку из очереди, начать её обработку и вернуть в пул для переиспользования.
        void getNewPoint_()
        {
        	ACTOR_PRESENT_S(p);
            Point* pPoint = actual.get();
            p.HandlingPoint = pPoint;
            startHandling_();
            store.put(pPoint);
        }

        // Переход из состояния Ожидание точек в Отрабатывается: точке назначается t = 0.
        void startHandling_()
        {
        	ACTOR_PRESENT_S(p);
            p.State = Trajectory_States::HANDLING;
            p.HandlingPoint->t = 0;
        }
    public:
        // Добавление новой точки траектории в очередь.
        bool approximer(Init& init_, long_signal_t start_time_)
        {
            Point* pointTmp = nullptr;
            if (store.count() > 0)
            {
                pointTmp = store.get();
            }
            else
            {
                return false;
            }

            // Находим профиль по номеру из init_.profile_type.
            profile_t* prof = findProfile(init_.profile_type);
            if (!prof)
            {
                store.put(pointTmp);
                return false;
            }

            pointTmp->params = &init_;
            pointTmp->done = false;
            prof->Approximer(*pointTmp, init_);
            pointTmp->time_start = start_time_;
            actual.put(pointTmp);
            return true;
        }
    private:
        // Поиск профиля в словаре по номеру.
        profile_t* findProfile(parameter_t profile_type)
        {
            profile_t** pp = profileDict.find(profile_type);
            return pp ? *pp : nullptr;
        }
    public:
        // Обработка траектории: интерполяция текущей точки или переход к следующей.
        virtual void run(long_signal_t& out, ulong_signal_t dt)
        {
        	ACTOR_PRESENT_S(p);
        	p.ObserverPoint = *p.HandlingPoint;

            switch (p.State)
            {
                case Trajectory_States::HANDLING:
                {
                    profile_t* prof = findProfile(p.HandlingPoint->params->profile_type);
                    if (prof)
                    {
                    	prof->Interpolator(*(p.HandlingPoint), dt);
                    }

                    out = p.HandlingPoint->out;

                    // Траектория точки завершена (достигнута целевая скорость) -
                    // переходим в ожидание, чтобы подхватить следующую точку из очереди.
                    if (p.HandlingPoint->done)
                    {
                        p.State = Trajectory_States::AWAITING;
                    }
                    break;
                }
                case Trajectory_States::AWAITING:
                    if (checkForNewPoints())
                    {
                    	getNewPoint_();
                    }
                    break;
            }
        }

        virtual void finish()
        {

        }

        void regvar_present(robo::cstr _name)
        {

        }

        void regvar_conf(robo::cstr _name)
        {

        }

        void do_regvar_present(void)
        {

        }

        void do_regvar_conf(void)
        {

        }

    };
}
#endif
