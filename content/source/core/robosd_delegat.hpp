#ifndef robosd_delegat_hpp
#define robosd_delegat_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_lambda.hpp"
namespace robo {
    namespace delegat {

        template <typename R, typename ... Args> class ROBO_EXPORT base {
        public:
            virtual R operator ()(Args... args) = 0;
			virtual ~base(void){} 
        };

		template <typename R, typename ... Args> class ROBO_EXPORT dummy: public base<R, Args...> {
		public:
            virtual R operator ()(Args... args) {};
		};

        template <class B, typename R, typename ... Args> class ROBO_EXPORT simple : public B {
            R(*delegat_) (Args... args);
        public:
            virtual R operator ()(Args... args) {
                ROBO_APP_ASSERT(delegat_);
                return delegat_(args...);
            }
            simple(void (*_delegat) (Args ... args))
                : delegat_(_delegat) {
            }

        };

        template <class B, typename R, typename ... Args> class ROBO_EXPORT uni : public B {
            void* instance_;
            R(*delegat_) (void* _instance, Args... args);
        public:
            virtual R operator ()(Args... args) {
                ROBO_APP_ASSERT(delegat_);
                return delegat_(instance_, args...);
            }
            uni(void* _instance, R(*_delegat) (void*, Args ... args))
                : instance_(_instance)
                , delegat_(_delegat) {
            }
        };

        template <class B, typename R, typename ... Args> class ROBO_EXPORT lambda : public B {
            ::robo::lambda< R(Args... args)> lambda_;
        public:
            virtual R operator ()(Args... args) {
                return lambda_(args...);
            }
            lambda(const ::robo::lambda< R(Args... args) > &  _lambda)
                : lambda_(_lambda) {
            }
        };

        template <class B, class C, typename R, typename ... Args> class ROBO_EXPORT member : public B {
            C* instance_;
            R(C::* member_) (Args... args);
        public:
            virtual R operator ()(Args... args) {
                ROBO_APP_ASSERT(instance_ != nullptr);
                return (instance_->*member_)(args...);
            }
            member(C* _instance, R(C::* _member) (Args ... args))
                : instance_(_instance)
                , member_(_member) {
            }
        };


//        template <typename R, typename ... Args> class ROBO_EXPORT ssimple :
  //          public simple< base< R, Args... >,  R, Args... > {
    //    };

        template <typename R, typename ... Args> class ROBO_EXPORT ssimple 
            : public  simple < base< R, Args... > , R, Args... > {
        public:
            ssimple(void (*_delegat) (Args ... args))
                : simple < base< R, Args... >, R, Args... >(_delegat) {
            }
        };

        template <typename R, typename ... Args> class ROBO_EXPORT suni
            : public  uni < base< R, Args... >, R, Args... > {
        public:
            suni(void* _instance, R(*_delegat) (void*, Args ... args))
                : uni < base< R, Args... >, R, Args... >(_instance, _delegat) {
            }
        };

        template <typename R, typename ... Args> class ROBO_EXPORT slambda
            : public  lambda < base< R, Args... >, R, Args... > {
        public:
            slambda(::robo::lambda< R(Args... args) > _lambda)
                : lambda < base< R, Args... >, R, Args... >(_lambda) {
            }
        };


        template <class C, typename R, typename ... Args> class ROBO_EXPORT smember
            : public  member < base<R, Args... >, C, R, Args... > {
        public:
            smember(C* _instance, R(C::* _member) (Args ... args))
                : member < base< R, Args... >,C, R, Args... >(_instance, _member) {
            }
        };
    }
}
#endif