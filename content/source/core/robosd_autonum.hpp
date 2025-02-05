#ifndef robosd_autonum_hpp
#define robosd_autonum_hpp
#include "core/robosd_system.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
namespace robo {
	namespace delegat{
		namespace autonum {
			class ROBO_EXPORT receicledbin {
				template <typename B, typename R, typename ... Args> friend class fabric;
				class ROBO_EXPORT ref {
					friend class receicledbin;
					template <typename B, typename R, typename ... Args> friend class autonum_t;
					int used_ = 0;
					bool isfrontend_;
					typedef ::robo::list::unsorted<ref> rblist;
					typedef rblist::ref rbref;
					struct core{
						core(::robo::cstr _lbl) : lbl(_lbl) {}
						rblist rbin;
						::robo::cstr lbl;
						int counter_ = 0;
						void inc(void) {
							robo_infolog("\t\t%s autonum delegat ++ (%d)", lbl, ++counter_);
						}
						void dec(void) {
							robo_infolog("\t\t%s autonum delegat -- (%d)", lbl, --counter_);
						}
					};
					static core& frontend_core_(void);
					static core& backend_core_(void);
					rbref rbref_;
					void release_(void);
					static void clean_(rblist& _list);
				protected:
					virtual void use(void) {
						used_++;
					}
					virtual void unuse(void) {
						if (used_ > 0) {
							used_--;
							if (used_ == 0) {
								release_();
							}
						}
					}
					ref(void)
						: rbref_(*this)
						#if ROBO_SYSTEM_ENABLED
						, isfrontend_(system::env::is_frontend())
						#else
						, isfrontend_(true)
						#endif
						{
						if (isfrontend_) {
							frontend_core_().inc();
						}
						else {
							backend_core_().inc();
						}
					}
					virtual ~ref(void) {
						if (isfrontend_) {
							frontend_core_().dec();
						}
						else {
							backend_core_().dec();
						}
					}

				public:

				};
			public:
				//#if ROBO_SYSTEM_ENABLED
				//friend class ::robo::system;
				//#endif
				static void frontend_clean(void);
				static void backend_clean(void);
			};

			template <typename B, typename R, typename ... Args> class ROBO_EXPORT fabric {
				//friend class simple;
				//friend class uni;
				//friend class lambda;

			public:
				class ROBO_EXPORT ref : public B, public receicledbin::ref {
				protected:
					template <typename ... xArgs> ref(xArgs ... xargs)
						: B(xargs...)
						, receicledbin::ref() {}
				public:
					virtual void attach(void) {
						B::attach();
						use();
					};
					virtual void dettach(void) {
						B::dettach();
						unuse();
					}
				public:
				};

				class ROBO_EXPORT simple
					: public  ::robo::delegat::simple <ref, R, Args... > {
					template<typename ... xArgs> simple(R(*_delegat) (Args ... args), xArgs ... xargs)
						: ::robo::delegat::simple <  ref, R, Args... >(_delegat, xargs...) {}
				public:
					template<typename ... xArgs> static simple* create(R(*_delegat) (Args ... args), xArgs ... xargs) {
						return new simple(_delegat, xargs...);
					}
				};

				class ROBO_EXPORT uni
					: public  ::robo::delegat::uni <  ref, R, Args... > {
					template<typename ... xArgs> uni(void* _instance, R(*_delegat) (void*, Args ... args), xArgs ... xargs)
						: ::robo::delegat::uni <  ref, R, Args... >(_instance, _delegat, xargs...) {}
				public:
					template<typename ... xArgs> static uni* create(void* _instance, R(*_delegat) (void*, Args ... args), xArgs ... xargs) {
						return new uni(_instance, _delegat, xargs...);
					}
				};

				class ROBO_EXPORT lambda
					: public  ::robo::delegat::lambda <  ref, R, Args... > {
					template<typename ... xArgs> lambda(::robo::lambda< R(Args... args) > _lambda, xArgs ... xargs)
						: ::robo::delegat::lambda <  ref, R, Args... >(_lambda, xargs...) {}
				public:
					template<typename ... xArgs> static lambda* create(::robo::lambda< R(Args... args) > _lambda, xArgs ... xargs) {
						return new lambda(_lambda, xargs...);
					}
				};

				template<class C> class ROBO_EXPORT member
					: public  ::robo::delegat::rmember <  ref, C, R, Args... > {
					template<typename ... xArgs> member(C& _instance, R(C::* _member) (Args ... args), xArgs ... xargs)
						: ::robo::delegat::rmember <  ref, C, R, Args... >(_instance, _member, xargs...) {}
				public:
					template<typename ... xArgs> static member* create(C& _instance, R(C::* _member) (Args ... args), xArgs ... xargs) {
						return new member(_instance, _member, xargs...);
					}
				};
				
				template<typename ... xArgs> static ref* create(R(*_delegat) (Args ... args), bool _unused, xArgs ... xargs) {
					ROBO_UNUSED(_unused);
					return simple::create(_delegat, xargs...);
				}
				template<typename ... xArgs> static ref* create(void* _instance, R(*_delegat) (void*, Args ... args), xArgs ... xargs) {
					return uni::create(_delegat, xargs...);
				}
				template<typename ... xArgs> static ref* create(::robo::lambda< R(Args... args) > _lambda, xArgs ... xargs) {
					return lambda::create(_lambda, xargs...);
				}
				template<class C, typename ... xArgs> static ref* create(C& _instance, R(C::* _member) (Args ... args), xArgs ... xargs) {
					return member<C>::create(_instance, _member, xargs...);
				}
				template<class C, typename ... xArgs> static ref* create(ref* _ref) {
					return _ref;
				}
			};

			template <typename R, typename ... Args> class ROBO_EXPORT performer : public ::robo::delegat::ref<R, Args...> {
			protected:
			public:
				virtual void attach(void) {};
				virtual void dettach(void) {};
				virtual R operator ()(Args... args) = 0;
			};
		}
		template <typename R, typename ... Args>
		using autonum_fabric = autonum::fabric < autonum::performer<R, Args...>, R, Args... >;
	}
}
#endif