#ifndef robosd_delegat_hpp
#define robosd_delegat_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_lambda.hpp"
namespace robo {
    namespace delegat {
        template <typename R, typename ... Args> class ROBO_EXPORT ref {
        protected:
            virtual ~ref(void) {}
        public:
            virtual R operator ()(Args... args) = 0;
        };

        template <typename R, typename ... Args> class ROBO_EXPORT dummy : public ref<R, Args...> {
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
        protected:
            template<typename ...xArgs> simple( R(*_delegat) (Args ... args), xArgs... xargs)
                : B( xargs... ), delegat_(_delegat) {
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
        protected:
            template<typename ... xArgs>  uni(void* _instance, R(*_delegat) (void*, Args ... args), xArgs... xargs)
                : B(xargs...)
                , instance_(_instance)
                , delegat_(_delegat) {
            }
        };

        template <class B, typename R, typename ... Args> class ROBO_EXPORT lambda : public B {
            ::robo::lambda< R(Args... args)> lambda_;
        public:
            virtual R operator ()(Args... args) {
                return lambda_(args...);
            }
        protected:
            template<typename ... xArgs>  lambda(const ::robo::lambda< R(Args... args) > &  _lambda, xArgs... xargs)
                : B(xargs...)
                , lambda_(_lambda) {
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
        protected:
            template<typename ... xArgs>  member(C* _instance, R(C::* _member) (Args ... args), xArgs... xargs)
                : B(xargs...)
                , instance_(_instance)
                , member_(_member) {
            }
        };

        template <class B, class C, typename R, typename ... Args> class ROBO_EXPORT rmember : public B {
            C& instance_;
            R(C::* member_) (Args... args);
        protected:
            template<typename ... xArgs>  rmember(C& _instance, R(C::* _member) (Args ... args), xArgs... xargs)
                : B(xargs...)
                , instance_(_instance)
                , member_(_member) {}
        public:
            virtual R operator ()(Args... args) {
                return (instance_.*member_)(args...);
            }
        };

        /*template <class RB, typename R, typename ... Args> class  pointer {
            ref<R, Args...> * pointer;
            bool auto_;
            R operator ()(Args... args);
        };*/
        namespace owned {

            template <class B, typename R, typename ... Args>
            class  ROBO_EXPORT fabric {
            public:
                class ROBO_EXPORT simple
                    : public  ::robo::delegat::simple < B, R, Args... > {
                    void* operator new(size_t, ...) = delete;
                    void* operator new[](size_t, ...) = delete;
                public:
                    template<typename ... xArgs> simple(R(*_delegat) (Args ... args), xArgs ... xargs)
                        : ::robo::delegat::simple < B, R, Args... >(_delegat, xargs...) {}
                };

                class ROBO_EXPORT uni
                    : public  ::robo::delegat::uni < B, R, Args... > {
                    void* operator new(size_t, ...) = delete;
                    void* operator new[](size_t, ...) = delete;
                public:
                    template<typename ... xArgs> uni(void* _instance, R(*_delegat) (void*, Args ... args), xArgs ... xargs)
                        : ::robo::delegat::uni < B, R, Args... >(_instance, _delegat, xargs...) {}
                };


                class ROBO_EXPORT lambda
                    : public  ::robo::delegat::lambda < B, R, Args... > {
                    void* operator new(size_t, ...) = delete;
                    void* operator new[](size_t, ...) = delete;
                public:
                    template<typename ... xArgs> lambda(::robo::lambda< R(Args... args) > _lambda, xArgs ... xargs)
                        : ::robo::delegat::lambda < B, R, Args... >(_lambda, xargs...) {}
                };

                template <class C> class ROBO_EXPORT member
                    : public  ::robo::delegat::rmember < B, C, R, Args... > {
                    void* operator new(size_t, ...) = delete;
                    void* operator new[](size_t, ...) = delete;
                public:
                    template<typename ... xArgs> member(C& _instance, R(C::* _member) (Args ... args), xArgs ... xargs)
                        : ::robo::delegat::rmember < B, C, R, Args... >(_instance, _member, xargs...) {}
                };
            };
        }
        template <typename R, typename ... Args>
        using owned_fabric = owned::fabric< ref< R, Args... >, R, Args... >;
    }
}
#endif