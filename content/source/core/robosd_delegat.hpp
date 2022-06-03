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
            simple(R (*_delegat) (Args ... args))
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

        template <class B, class C, typename R, typename ... Args> class ROBO_EXPORT rmember : public B {
            C& instance_;
            R(C::* member_) (Args... args);
        public:
            virtual R operator ()(Args... args) {
                return (instance_.*member_)(args...);
            }
            rmember(C & _instance, R(C::* _member) (Args ... args))
                : instance_(_instance)
                , member_(_member) {}
        };

//        template <typename R, typename ... Args> class ROBO_EXPORT ssimple :
  //          public simple< base< R, Args... >,  R, Args... > {
    //    };

        template <typename R, typename ... Args> class ROBO_EXPORT ssimple 
            : public  simple < base< R, Args... > , R, Args... > {
        public:
            ssimple(R (*_delegat) (Args ... args))
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

        template <class C, typename R, typename ... Args> class ROBO_EXPORT srmember
            : public  rmember < base<R, Args... >, C, R, Args... > {
        public:
            srmember(C & _instance, R(C::* _member) (Args ... args))
                : rmember < base< R, Args... >, C, R, Args... >(_instance, _member) {}
        };

        /*
        * 
        * Так нельзя! удаление может произойти в контексте, отличном от создания а так нельзя
        namespace temporary {
            template< class C > class killer {
                C* ptr_;
            public:
                killer(C* _ptr): ptr_(_ptr) {}
                ~killer() { delete ptr_; }
            };

            template< class B, typename R, typename ... Args > class simple : public ::robo::delegat::simple<B, R, Args... > {
                friend class killer<simple>;
                virtual ~simple() {}
            public:
                virtual R operator ()(Args... args) {
                    killer<simple> k_(this);
                    return ::robo::delegat::simple<B, R, Args... >::operator()(args...);
                };
                simple(R(*_delegat) (Args ... args))
                    : ::robo::delegat::simple<B, R, Args... >(_delegat) {}
            };

            template< typename R, typename ... Args > class ssimple : public ::robo::delegat::ssimple<R, Args... > {
                friend class killer<ssimple>;
                virtual ~ssimple() {}
            public:
                virtual R operator ()(Args... args) {
                    killer<ssimple> k_(this);
                    return ::robo::delegat::ssimple<R, Args... >::operator()(args...);
                };
                ssimple(R(*_delegat) (Args ... args))
                    : ::robo::delegat::ssimple<R, Args... >(_delegat) {}
            };


            template< class B, typename R, typename ... Args > class uni : public ::robo::delegat::uni<B, R, Args... > {
                friend class killer<uni>;
                virtual ~uni() {}
            public:
                virtual R operator ()(Args... args) {
                    killer<uni> k_(this);
                    return ::robo::delegat::uni<B, R, Args... >::operator()(args...);
                };
                uni(void* _instance, R(*_delegat) (void*, Args ... args))
                    : ::robo::delegat::simple<B, R, Args... >(_instance, _delegat) {}
            };

            template< typename R, typename ... Args > class suni : public ::robo::delegat::suni<R, Args... > {
                friend class killer<suni>;
                virtual ~suni() {}
            public:
                virtual R operator ()(Args... args) {
                    killer<suni> k_(this);
                    return ::robo::delegat::suni<R, Args... >::operator()(args...);
                };
                suni(void* _instance, R(*_delegat) (void*, Args ... args))
                    : ::robo::delegat::suni<R, Args... >(_instance,_delegat) {}
            };

            template< class B, typename R, typename ... Args > class lambda : public ::robo::delegat::lambda<B, R, Args... > {
                friend class killer<lambda>;
                virtual ~lambda() {}
            public:
                virtual R operator ()(Args... args) {
                    killer<lambda> k_(this);
                    return ::robo::delegat::lambda<B, R, Args... >::operator()(args...);
                };
                lambda(::robo::lambda< R(Args... args) > _lambda)
                    : ::robo::delegat::lambda<B, R, Args... >( _lambda) {}
            };

            template< typename R, typename ... Args > class slambda : public ::robo::delegat::slambda<R, Args... > {
                friend class killer<slambda>;
                virtual ~slambda() {}
            public:
                virtual R operator ()(Args... args) {
                    killer<slambda> k_(this);
                    return ::robo::delegat::slambda<R, Args... >::operator()(args...);
                };
                slambda(::robo::lambda< R(Args... args) > _lambda)
                    : ::robo::delegat::slambda<R, Args... >(_lambda) {}
            };

        }

        */
    }
}
#endif