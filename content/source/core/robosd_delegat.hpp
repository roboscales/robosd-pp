#ifndef robosd_delegat_hpp
#define robosd_delegat_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_lambda.hpp"
namespace robo {
	namespace delegat {

		template <typename R, typename ... Args> class ROBO_EXPORT base{
        public:
            virtual R operator ()(Args... args) =0;
        };

        template <typename R, typename ... Args> class ROBO_EXPORT simple : public base<R, Args... > {
            R (* delegat_) (Args... args);
        public:
            virtual R operator ()(Args... args){
                ROBO_APP_ASSERT(delegat_);
                return delegat_ (args...);
            }
            simple( void (* _delegat) (Args ... args) )
                : delegat_(_delegat){
            }

        };

        
        template <typename R, typename ... Args> class ROBO_EXPORT uni : public base<R, Args... > {
            void * instance_;
            R (* delegat_) (void * _instance, Args... args);
        public:
            virtual R operator ()(Args... args){
                ROBO_APP_ASSERT(delegat_);
                return delegat_ (instance_, args...);
            }
            uni( void * _instance, R (* _delegat) (void *, Args ... args) )
                : instance_(_instance)
                , delegat_(_delegat){
            }
        };

        template <typename R, typename ... Args> class ROBO_EXPORT lmbda : public base<R, Args... > {
            ::robo::lambda< R(Args... args)> lambda_;
        public:
            virtual R operator ()(Args... args) {
                return lambda_(args...);
            }
            lmbda( ::robo::lambda< R(Args... args) > _lambda )
                : lambda_(_lambda) {
            }
        };

        template <class C, typename R, typename ... Args> class ROBO_EXPORT member : public base<R, Args... > {
            C &  instance_;
            R( C:: * member_) ( Args... args);
        public:
            virtual R operator ()(Args... args) {
                return (instance_.*member_)(args...);
            }
            member(C & _instance, R ( C::* _member) ( Args ... args))
                : instance_(_instance)
                , member_(_member) {
            }
        };

    }
}
#endif