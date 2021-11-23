#ifndef __modes_hpp
#define __modes_hpp
#include "core/robosd_common.h"
namespace robo{
	namespace models{
		class ROBO_EXPORT block{
		private:
			void update_begin_(void);
			void update_end_(void);
			int update_count_;
		protected:
			friend class guard;
			class ROBO_EXPORT guard{
				friend class block;
				block & owner_;
			public:
				guard(block & _owner);
				~guard();
			};
			robo_string_t ini_section;
			float model_period_sec;
			virtual void reconfig(void) = 0;
			virtual bool setup(void) = 0;
		public:
			virtual void run(void) = 0;

			bool configure(const robo_string_t _ini_section, float _model_period_sec);
			template <typename T> class parametr{
				block & owner_;
				T & value_;
			public:
				parametr(block & _owner, T &_value)
					: owner_(_owner)
					, value_(_value){
				}
				parametr & operator = (const T & _src){
					guard g__(owner_);
					value_ = _src;
					return *this;
				}
				operator const T & () const{ return value_; }
			};
		};		
	}
}
#endif