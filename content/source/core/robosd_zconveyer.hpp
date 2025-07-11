#ifndef __robo_zconveyer_hpp
#define  __robo_zconveyer_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_matrix.hpp"
namespace robo {
	template <typename M> struct zconveyer_t {
        typedef  typename M::real real;
        struct counter_s;

        class signal_s{
        public:
            counter_s& counter;
            const struct config_s {
                int size;
            } ;
        private:
            const config_s& config_;
        public:

            template <typename T> const   T& config(void) {
                return reinterpret_cast <const  T&>(config_);
            }
            #define Z_CONFIG_S(s) const config_s& s= signal_s::template config<config_s>()

            signal_s(counter_s& _counter, const config_s & _config)
                : counter(_counter)
                , config_(_config){
                Size = config_.size;
                buffer =  new real[Size];
                clean();
                //counter.push(*this);
            }
            virtual ~signal_s(void) {
                if (buffer) delete[] buffer;
            }
            void run(void);
            real * buffer = nullptr;

            virtual void doclean(void) {};
            real value(void) { 
                M::assert(Last >= 0);
                return buffer[Last];
            }
            virtual void clean(void) {
                First = -1;
                Last = -1;
                Current = -1;
                Count = 0;
                Interval1Beg = -1;
                Interval1Len = -1;
                Interval2Beg = -1;
                Interval2Len = -1;
                TotalCount = 0;
                Index = 0;
                doclean();
            }

            int Interval1Beg = -1;
            int Interval1Len = -1;
            int Interval2Beg = -1;
            int Interval2Len = -1;
            int First = -1;
            int Last = -1;
            int Current = -1;
            int Count = 0;
            int TotalCount = 0;
            int Index = 0;
            int Size;
            void put(const real & value) {
                if (Current == -1) {
                    Current = 0;
                }
                buffer[Current] = value;
                if (First == -1) {
                    First = 0;
                }
                Last = Current;
                if (Current < Size - 1) {
                    Current++;
                }
                else {
                    Current = 0;
                }
                if (Current == First) {
                    if (First < Size - 1) {
                        First++;
                    }
                    else {
                       First = 0;
                    }
                }
                TotalCount++;
                if (Count < Size) {
                    Count++;
                }
            }
            void update_interval(int _count) {
                int _First;
                if (_count > Count) {
                    _count = Count;
                }

                _First = Last - _count + 1;

                if (_First >= 0) {
                    Interval1Beg = _First;
                    Interval1Len = Last - _First + 1;
                    Interval2Len = 0;
                }
                else {
                    _First = _First + Size;
                    Interval1Beg = _First;
                    Interval1Len = Size - _First;
                    Interval2Beg = 0;
                    Interval2Len = Last + 1;
                }
            }
        };

        class actor_s: public signal_s {
        public:
            actor_s* next_ = nullptr;
            struct config_s {
                signal_s::config_s signal;
            };
            actor_s(counter_s& _counter, signal_s & _input, const config_s& _config)
                : signal_s(_counter, _config.signal)
                , input(_input)
            {
                _counter.actors.push(*this);
            }
            virtual ~actor_s(void) {
            }

            virtual void doit(void) = 0;

            signal_s & input;
            typedef robo::list::unidir::base_t<actor_s> list;

       };

        struct sample_s;
        struct counter_s  {
            counter_s* next_ = nullptr;
            int presc;
            int period_tick;
            typename actor_s::list actors;
            typedef robo::list::unidir::base_t<counter_s> list;
            sample_s& sample;
            counter_s(int _presc, int _offset, sample_s& _sample) 
                : presc(_presc)
                , period_tick(_offset)
                , sample(_sample)
            {
                _sample.counters.push(*this);
            }
            void run(void) {
                actor_s* s;
                period_tick--;
                if (period_tick <= 0) {
                    for (s = actors.first(); s; s = s->next_) {
                        s->doit();
                    }
                    period_tick = presc;
                }
            }
           

        };

        struct sample_s  {
            sample_s* next_;
            int period;
            typedef robo::list::unidir::base_t<sample_s> list;
            typename counter_s::list  counters;
            
            sample_s(int _period, zconveyer_t& _conveyer): period(_period){
                _conveyer.samples.push(*this);
            }

            void run(void) {
                counter_s * p;
                for (p = counters.first(); p; p = p->next_) {
                    p->run();
                }
            }
        };

        sample_s* get_sample(int _period) {
            for (event_s* p = events.firs(); p; p = p->next_) {
                if (p->period == _period)
                    return p;
            }
            return nullptr;
        }
        sample_s* default_sample = nullptr;
        counter_s *  get_counter(int _period) {
            sample_s * sample;
            counter_s * counter = nullptr;
            sample = get_sample(_period);
            if (sample == nullptr) {
                if (default_sample == nullptr)
                    default_sample =new sample(0);
                sample = default_sample;
            }
            for (counter = sample->counters.first(); counter; counter = counter->next_) {
                if (counter->id == period) {
                    break;
                }
            }
            if (counter == nullptr) {
                counter = new counter(period,0,*sample);
            }
            return counter;
        }


        typename sample_s::list samples;
        //int sample_index = 0;
        
        //char* str_buf;
        //int str_len;
//        event_s* first_event;
        //event_s* last_event;
        //sample_s* default_event;
        //signal_s* first_signal;
        //signal_s* last_signal;
        //signal_s * input;
        virtual ~zconveyer_t(void) {
            if (default_sample) {
                delete default_sample;
            }
        }

        struct scale_s :public actor_s {
            struct config_s {
                actor_s::config_s actor;
                int delay = 0;
                real offset = 0;
                real gain = 0;
            };
            scale_s(counter_s& _counter,  signal_s& _input, const config_s& _config) 
                : actor_s(_counter,_input,_config.actor) {
            }
            virtual void doit(void) {
                Z_CONFIG_S(s);
                real x = 0;
                if (input.Count > s.delay) {
                    int ix = Last - s.delay;
                    if (ix < 0)
                        ix += (input.Size);
                    x = input.buffer[ix];
                    x = x * s.gain + s.offset;
                }
                actor_s::put(x);
            }
        };

        struct delay_s :public actor_s {
            struct config_s {
                actor_s::config_s actor;
                int delay;
            };

            delay_s(counter_s& _counter,  signal_s& _input, const config_s& _config)
                : actor_s(_counter, _input, _config.actor) {
            }

            virtual void doit(void) {
                Z_CONFIG_S(s);
                real x = 0;
                if (input.Count > s.delay) {
                    int ix = input.Last - s.delay;
                    if (ix < 0)
                        ix += (input.Size);
                    x = input.buffer[ix];
                }
                put(x);
            }
        };

        struct diff_s :public actor_s {
            typedef actor_s::config_s config_s;
            signal_s& y;
            diff_s(counter_s& _counter, signal_s& _input, signal_s& _y, const config_s& _config)
                : actor_s(_counter, _input, _config), y(_y) {
            }

            virtual void doit(void) {
                Z_CONFIG_S(s);
                real x = input.value();
                
                put(x-y.value());
            }
        };

        struct sqr_s :public actor_s {
            typedef actor_s::config_s config_s;
            sqr_s(counter_s& _counter,  signal_s& _input, const config_s& _config)
                : actor_s(_counter, _input, _config) {
            }
            virtual void doit(void) {
                real x = 0;
                if (input.Count > 0) {
                    x = input.buffer[input.Last];
                }
                put(x*x);
            }
        };
        struct sqrt_s :public actor_s {
            typedef actor_s::config_s config_s;
            sqrt_s(counter_s& _counter,  signal_s& _input, const config_s& _config)
                : actor_s(_counter, _input, _config) {
            }
            virtual void doit(void) {
                real x = 0;
                if (input.Count > 0) {
                    x = input.buffer[input.Last];
                }
                put(M::sqrt(x));
            }
        };
        
        struct nconfig_s {
            typename actor_s::config_s actor;
            int len;
        };

        struct slide_mean_s :public actor_s {
            typedef nconfig_s config_s;
            real sum = 0;
            real prev = 0;
            slide_mean_s(counter_s& _counter,  signal_s& _input, const config_s& _config)
                : actor_s(_counter, _input, _config.actor) {
            }
            virtual void doit(void) {
                //real x = 0;
                Z_CONFIG_S(s);
                int cnt = input.Count;
                if (cnt < s.len) {
                    sum += input.buffer[input.Last];
                    put(sum / cnt);
                }
                else {
                    int ix = input.Last - s.len + 1;
                    if (ix < 0)
                        ix += (input.Size);

                    sum -= prev;
                    sum += input.buffer[input.Last];
                    prev = input.buffer[ix];
                    put(sum / s.len);
                }
            }
        };
        

        struct slide_sum_s :public actor_s {
            typedef nconfig_s config_s;
            real sum = 0;
            real prev = 0;
            slide_sum_s(counter_s& _counter,  signal_s& _input, const config_s& _config)
                : actor_s(_counter, _input, _config.actor) {
            }
            virtual void doit(void) {
                //real x = 0;
                Z_CONFIG_S(s);
                int cnt = input.Count;
                if (cnt < s.len) {
                    sum += input.buffer[input.Last];
                    put(sum );
                }
                else {
                    int ix = input.Last - s.len + 1;
                    if (ix < 0)
                        ix += (input.Size);

                    sum -= prev;
                    sum += input.buffer[input.Last];
                    prev = input.buffer[ix];
                    put(sum );
                }
            }
        };
        
        struct var0_s :public actor_s {
            typedef actor_s::config_s config_s;
            
            const signal_s & mean;
            
            var0_s(counter_s& _counter,  signal_s& _input, const signal_s& _mean, const config_s& _config)
                : actor_s(_counter, _input, _config), mean(_mean) {
            }

            virtual void doit(void) {
                real x = 0;
                if (input.Count > 0) {
                    x = input.buffer[input.Last] - mean.buffer[mean.Last];
                }
                put(x * x);
            }
        };

        struct disp_s :public actor_s {
            typedef nconfig_s config_s;
            real sum = 0;
            real prev = 0;
            disp_s(counter_s& _counter,  signal_s& _input, const config_s& _config)
                : actor_s(_counter, _input, _config.actor) {
            }
            virtual void doit(void) {
                Z_CONFIG_S(s);
                int cnt = input.Count;
                if (cnt < s.len) {
                    sum += input.buffer[input.Last];
                    if (cnt < 2) {
                        put(0);
                    }
                    else {
                        put(sum / (cnt - 1));
                    }
                }
                else {
                    int ix = input.Last - s.len + 1;
                    if (ix < 0)
                        ix += (input.Size);
                    sum -= prev;
                    sum += input.buffer[input.Last];
                    prev = input.buffer[ix];
                    put(sum / (s.len - 1) );
                }
            }
        };
        struct filter1_s :public actor_s {
            struct config_s {
                actor_s::config_s actor;
                real tau;
                real itau;
            };
            filter1_s(counter_s& _counter,  signal_s& _input, const config_s& _config)
                : actor_s(_counter, _input, _config.actor) {
            }
            virtual void doit(void) {
                Z_CONFIG_S(s);
                put(buffer[Last] * tau + itau * input.buffer[input.Last]);
            }
        };

        struct zero_ord_holder_s :public actor_s {
            typename actor_s::config_s actor;
            struct config_s {
            };
            zero_ord_holder_s(counter_s& _counter,  signal_s& _input, const config_s& _config)
                : actor_s(_counter, _input, _config.actor) {
            }
            virtual void doit(void) {
                real x = 0;
                if (input.Count > 0) {
                    x = input.buffer[input.Last];
                }                
                put(x);
            }
        };
        struct cubic_approxx_base_s :public actor_s {
            typedef nconfig_s config_s;

            cubic_approxx_base_s(counter_s& _counter,  signal_s& _input, const config_s& _config)
                : actor_s(_counter, _input, _config.actor) 
                ,A(_counter, _config.actor.signal)
                , B(_counter, _config.actor.signal)
                , C(_counter, _config.actor.signal)
                , D(_counter, _config.actor.signal)
            {
            }
            signal_s A;
            signal_s B;
            signal_s C;
            signal_s D;
        };

        struct cubic_approxx_s :public cubic_approxx_base_s {
            typedef nconfig_s config_s;

            cubic_approxx_s(counter_s& _counter, signal_s& _input, const config_s& _config)
                : cubic_approxx_base_s(_counter, _input, _config)
                , P(1, 4)
                , TT(4,_config.len)
                , ITT(_config.len,4)
                , Err (1, _config.len)
                , SY(1, _config.len)
                , TS (4, 4)
                , ITS(4, 4)
                {
                    for ( int i = 0; i < _config.len; ++i) {
                        real a;

                        TT.memo[0][i] = 1;

                        a = i;
                        TT.memo[1][i] = a;

                        a *= i;
                        TT.memo[2][i] = a;

                        a *= i;
                        TT.memo[3][i] = a;
                    }

                    TT.pinvto(ITT, TS, ITS);
                }
            matrix_t<M> Err;
            matrix_t<M> SY;
            matrix_t<M> TT;
            matrix_t<M> ITT;
            matrix_t<M> P;
            matrix_t<M> TS;
            matrix_t<M> ITS;


            virtual void doit(void) {
                int i,  k = 0;
                real err = 0, err2;
                Z_CONFIG_S(s);
                auto& Y = input;
                Y.update_interval(s.len);
                if (Y.Count >= s.len) {
                    for (i = Y.Interval1Beg; i < Y.Interval1Beg + Y.Interval1Len; ++i) {
                        SY.memo[0][k] = Y.buffer[i];
                        k++;
                    }
                    if (Y.Interval2Len > 0) {
                        for (i = Y.Interval2Beg; i < Y.Interval2Beg + Y.Interval2Len; ++i) {
                            SY.memo[0][k] = Y.buffer[i];
                            k++;
                        }
                    }

                    //ix = X.Last - s.len + 1;
                    //if (ix < 0)
                        //ix += (X.Size);

                    SY.multto(ITT, P);
                    P.multto(TT,Err);

                    for (i = 0; i < s.len; ++i) {
                        err2 = Err.memo[0][i] - SY.memo[0][i];
                        err2 *= err2;
                        err += err2;
                    }
                    A.put(P.memo[0][0]);
                    B.put(P.memo[0][1]);
                    C.put(P.memo[0][2]);
                    D.put(P.memo[0][3]);
                    put(err);
                }
                else {
                    A.put(0);
                    B.put(0);
                    C.put(0);
                    D.put(0);
                    put(0);
                }
            }
        };
        struct cubic_interp_s :public actor_s {
            typedef actor_s::config_s config_s;
            cubic_approxx_base_s& approox;
            cubic_interp_s(counter_s& _counter, signal_s& _input, cubic_approxx_base_s& _aprx, const config_s& _config)
                : actor_s(_counter, _input, _config), approox(_aprx){
            }

            void doit(void) {
                real y = 0;
                real tmp;
                real x = input.value();
                y += approox.A.value();
                y += approox.B.value()*x;
                tmp = (x * x);
                y += (approox.C.value() * tmp);
                y += (approox.D.value() * tmp * x);

                put(y);

            }
        };
 	};

    template <typename M> void zconveyer_t<M>::signal_s::run(void) {
        counter.sample.run();
    }
}

#endif
