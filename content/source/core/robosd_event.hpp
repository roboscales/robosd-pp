#ifndef robosd_event_hpp
#define robosd_event_hpp
#include "core/robosd_delegat.hpp"
#include "core/robosd_autonum.hpp"
namespace robo {
	template<typename R, typename ... Args > class ROBO_EXPORT event_t {
	public:
		friend class performer;
		class ROBO_EXPORT  performer {
		public:
			enum class priority { lo = -1, normal = 0, hi = 1 };
			typedef ::robo::list::sorted <performer, priority> list;
			typedef typename list::ref ref;
			friend class event_t;
		protected:
			ref ref_;
			bool once_;
		public:
			virtual void attach(void) {};
			virtual void dettach(void) { ref_.dettach(); };
			inline bool once(void) { return once_; }
			inline void set_once(bool _once) { once_ = _once; }
			inline bool cancel(void) {
				if (ref_.attached()) {
					dettach();
					return false;
				}
				else {
					return true;
				}
			}
			virtual  R  operator ()(Args..._args) = 0;
			performer(bool _once = false)
				: ref_(*this, priority::normal)
				, once_(_once) {};
			virtual ~performer(void) {};
			bool attach_to(event_t* _event, priority _priority = priority::lo) {
				ref_.set_key(_priority);
				if (!ref_.attached()) {
					ROBO_LBREAKN(ref_.attach_to(_event->performers_))
						attach();
				}
				return true;
			}

			//virtual bool temporary(void) { return false; }
		};

	private:
		typename performer::list performers_;
		template<typename T> 
			struct  result { 
				int t = {};
				result(void) {}
				result(const T & _t):t(_t) {}
				void run (performer* p, Args ... _arg) { t |= (int)(*p)(_arg...); }
				T value(void) { return (T)t;  }
			};
		template<> 	struct  result<void> {
				result(void) {}
				void run(performer* p, Args ... _arg) {  (*p)(_arg...); }
				void value(void) {}
			};
	public:
		inline bool used(void) { return performers_.count() > 0; }

		event_t(void) {

		}
		R raise(Args ... _arg) {
			typename event_t::performer::ref* _ref = performers_.first();
			result<R> res;
			while (_ref) {
				typename performer::ref* tmp = _ref;
				performer* p = &(tmp->owner());
				_ref = _ref->next();
				res.run(p, _arg...);
				//res.or( (*p)(_arg...) );
				if (p->once()) {
					p->dettach();
				}
			}
			return res.value();
		}
		typedef ::robo::delegat::owned::fabric < performer, R, Args...> owned;
		typedef ::robo::delegat::autonum::fabric<performer, R, Args...> autonum;
	};

	namespace events {
		typedef event_t<void, const uint8_t*, size_t > on_receive;
		typedef event_t<void> on_panic;
	}
}
#endif