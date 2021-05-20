#ifndef __robosd_lambda_h
#define __robosd_lambda_h


#include "core/robosd_common.hpp"
#include <algorithm>

namespace robo{

	template <typename T> class  ROBO_EXPORT lambda {};

	template <typename Out, typename ...In>  class  ROBO_EXPORT lambda<Out(In...)>
	{
		class ROBO_EXPORT context{
			uint8_t * p_ = nullptr;
			int sz_;
			int used_ = 0;
			Out(*run_)(context *, In...) = nullptr;
		public:
			#if ROBO_MEMORY_HEAP_ENABLED == 1
			ROBO_REDECLARE_NEW 
			#endif

			~context(void){
				#if ROBO_MEMORY_HEAP_ENABLED == 1
				robo_mem_free((robo_mem_t) p_);
				#else
				delete[] p_;
				#endif
			}
			const void * p(void){ return p_; }
			context * use(void){
				used_++;
				return this;
			}

			template<typename T> static context * query(T const & _context){
				context * ctx = new context;
				ROBO_APP_ASSERT(ctx != nullptr);
				ctx->sz_ = sizeof(T);
				ctx->p_ = new uint8_t[ctx->sz_];
				std::copy_n((uint8_t*)&_context, ctx->sz_, (uint8_t*)ctx->p_);
				ctx->used_++;
				ctx->run_ = [](context *_context, In... arguments) -> Out
				{
					T * foo = (T *)(_context->p());
					return (*foo)(arguments...);
				};
				return ctx;
			}
			static void release(context * & _ctx){
				_ctx->used_--;
				if (_ctx->used_ <= 0){
					delete _ctx;
				}
				_ctx = nullptr;
			}
			Out run(In... in){
				return run_(this, in...);
			}
		};

	public:
		lambda()
		{
		}

		lambda(lambda<Out(In...)> const & _src)
		{
			if (_src.context_ != nullptr)

			context_ = _src.context_->use();
		}

		template<typename T>
		lambda(T const & _context)
		{
			context_ = context::query(_context);
		}

		~lambda()
		{
			if (context_ != nullptr)
				context::release(context_);
			context_ = nullptr;
		}

		lambda<Out(In...)> &operator =(lambda<Out(In...)> const &_src)
		{
			if (context_ != nullptr)
				context::release(context_);
			context_ = nullptr;
			if (_src.context_ != nullptr)
				context_ = _src.context_->use();
			return *this;
		}


		operator bool()
		{
			return context_ != nullptr;
		}
		Out operator()(In ... in)
		{
			return context_->run(in...);
		}

	private:
		context * context_ = nullptr;
	};
}
#endif
