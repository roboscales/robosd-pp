#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_string.hpp"
#include <algorithm>

namespace robo {
	int32_t hash(cstr _src, int32_t _begin) {
		const int32_t p = 31;
		const int32_t m = 1000000009;
		int32_t hash_value = _begin;
		int32_t p_pow = 1;
		const char_t* c;
		for (c = _src; *c; c++) {
			hash_value = (hash_value + (*c - 'a' + 1) * p_pow) % m;
			p_pow = (p_pow * p) % m;
		}
		return hash_value;
	}

	unsigned short fast_hash(cstr _src, unsigned short _begin) {
		unsigned short x = _begin;
		const char_t* c;
		for (c = _src; *c; c++) {
			x += 44111 * (*c);
		}
		return x;
	}
	char const* fault_file_ = nullptr;
	char const* fault_function_ = nullptr;
	int fault_line = 0;

	void crash(char const* _file, char const* _function, int _line) {
		fault_file_ = _file;
		fault_function_ = _function;
		fault_line = _line;
		#if ROBO_APP_ENV_ENABLED == 1
		system::env::abort();
		#else
		while (true) {}
		#endif
	}
}

#if ROBO_APP_SYSTEM_ENABLED == 1

#if ROBO_APP_ALLOC_ENABLED == 1
#ifndef ROBO_SYSTEM_ALLOCATOR_WORD_TYPE
#define ROBO_SYSTEM_ALLOCATOR_WORD_TYPE uint32_t
#endif

#ifndef ROBO_SYSTEM_ALLOCATOR_SIZE_TYPE
#define ROBO_SYSTEM_ALLOCATOR_SIZE_TYPE uint16_t
#endif

#ifndef ROBO_SYSTEM_ALLOCATOR_SIZE
#define ROBO_SYSTEM_ALLOCATOR_SIZE 1024
#endif

#ifndef ROBO_SYSTEM_ALLOCATOR_WORD_SIZE_BITS
#define ROBO_SYSTEM_ALLOCATOR_WORD_SIZE_BITS 2
#endif
#endif

namespace robo {
	system& system::instance_(void) {
		static system instance__;
		return instance__;
	}
	#if ROBO_APP_ALLOC_ENABLED == 1
	class allocator {
		friend class system;
		typedef ROBO_SYSTEM_ALLOCATOR_WORD_TYPE word;
		typedef ROBO_SYSTEM_ALLOCATOR_SIZE_TYPE size;
		enum { size_ = (ROBO_SYSTEM_ALLOCATOR_SIZE>>ROBO_SYSTEM_ALLOCATOR_WORD_SIZE_BITS), bits_ = ROBO_SYSTEM_ALLOCATOR_WORD_SIZE_BITS };
		struct {
			size_t useful = 0;
			size_t used = 0;
			struct {
				size_t query = 0;
				size_t success = 0;
			} total;
			struct {
				size_t query = 0;
				size_t success = 0;
			} count;
		} statistic_;

		struct handler {
			union {
				struct {
					size prev_offset;
					size size;
				};
				word header;
			};
			void* memo(void) { return (void*)(&header + 1); }
		};

		word  memo_[size_ + 1];
		handler* top_ = reinterpret_cast<handler*>(memo_);
		static allocator instance_;
		size memo_top_ = 0;
		allocator(void) {
			std::fill_n(memo_, size_ + 1, 0);
		}
		size offset_(size_t _sz) {
			size b = (size)bits_;
			size deseired = (size)(_sz >> b);
			size tmp = (size)(deseired << b);
			if (tmp < _sz) {
				deseired++; //данные не выровнены
			}
			return (size) (deseired + 1);
		}
		void* query(size_t _sz) {

			size offset = offset_(_sz);

			size next_top = (size)(memo_top_ + offset);

			statistic_.total.query += _sz;
			statistic_.count.query++;

			if (size_ >= next_top) {
				handler* next_ = reinterpret_cast<handler*>(memo_ + next_top);
				next_->prev_offset = offset;
				next_->size = 0;
				top_->size = (size)_sz;
				memo_top_ = next_top;
				statistic_.useful += _sz;
				statistic_.used += offset * sizeof(word);
				statistic_.total.success += _sz;
				statistic_.count.success++;
				void* memo = top_->memo();
				top_ = next_;
				return memo;
			}
			else {
				return nullptr;
			}
		}

		void release_(handler* _handler) {
			size offset = offset_(_handler->size);

			statistic_.useful -= _handler->size;
			statistic_.used -= offset * sizeof(word);

			_handler->size = 0;
		}

		void release(void* _memo) {
			handler* tmp = (handler*)(((word*)_memo) - 1);

			if (tmp != top_ - top_->prev_offset) {
				//todo проверить
				release_(tmp);
			}
			else {
				do {
					if (tmp->size != 0) {
						release_(tmp);
					}
					top_ = tmp;
					memo_top_ = (size)(reinterpret_cast<word*>(top_) - memo_);
					if (memo_top_ == 0) break;
					tmp = tmp - tmp->prev_offset;
				} while (tmp->size == 0);
			}
		}

		size_t owned(void* _memo) {
			int sz = (int)(reinterpret_cast<word*>(_memo) - memo_);
			if (sz > 0 && sz < size_) {
				handler* tmp = (handler*)(((word*)_memo) - 1);
				return  tmp->size;
			}
			else {
				return 0;
			}
		}
	};

	allocator allocator::instance_;
	#endif

	void* system::enter_() {
		#if ROBO_APP_ENV_ENABLED == 1
		if (state_ == state::enabled) {
			if (env::is_frontend()) {
				void* context_ = env::enter();
				guest_count_++;
				return context_;
			}
			else {
				if (lock_count_ == 0) {
					env::lock();
				}
				lock_count_++;
			}
		}
		#endif
		return nullptr;
	}

	void system::leave_(void* context_) {
		#if ROBO_APP_ENV_ENABLED == 1
		if (state_ == state::enabled) {
			if (env::is_frontend()) {
				guest_count_--;
				env::leave(context_);
			}
			else {
				if (lock_count_ > 0) {
					lock_count_--;
					if (lock_count_ == 0) {
						env::unlock();
					}
				}
			}
		}
		#endif
	}

	system::guard::guard(void) {
		context_ = system::instance_().enter_();
	}

	system::guard::~guard(void) {
		system::instance_().leave_(context_);
	}

	void* system::critical_enter_(void) {
		#if ROBO_APP_ENV_ENABLED == 1
		//быстрым процессам сдесь делать нечего - это разборки между потоками "фронткнд"
		ROBO_APP_ASSERT(env::is_frontend());
		return system::env::critical_enter();
		#else
		return nullptr;
		#endif
	}

	void system::critical_leave_(void* _context) {
		#if ROBO_APP_ENV_ENABLED == 1
		ROBO_APP_ASSERT(env::is_frontend());
		system::env::critical_leave(_context);
		#else
		ROBO_UNUSED(_context);
		#endif
	}

	system::critical::critical(void) {
		context_ = system::instance_().critical_enter_();
	}

	system::critical::~critical(void) {
		system::instance_().critical_leave_(context_);
	}

	system::system(void) {
		state_ = state::enabled;
	}

	system::~system(void) {
		state_ = state::unknown;
	}

	#if ROBO_APP_ENV_ENABLED == 1

	system::lazzyboy::lazzyboy(void)
		: sleep_us_(system::env::time_us()) {}

	robo::time_us_t system::lazzyboy::idle_us(void) {
		return env::time_us() - sleep_us_;
	}

	#else
	system::lazzyboy::lazzyboy(void)
		: sleep_us_(0) {}
	robo::time_us_t	 system::lazzyboy::idle_us(void) {
		return  0;
	}
	#endif

	system::lazzyboy::~lazzyboy(void) {}


	system::fall::fall(void) {
		#if ROBO_APP_ENV_ENABLED == 1
		env::fall();
		#endif
	}

	system::fall::~fall(void) {
		#if ROBO_APP_ENV_ENABLED == 1
		env::comeback();
		#endif
	}

	#if ROBO_APP_ALLOC_ENABLED == 1
	void* system::mem_alloc_(size_t _sz) {
		size_t _tsz;
		void* ptr;
		{
			guard g__;
			if (env::is_backend()) {
				ptr = allocator::instance_.query(_sz);
				ROBO_APP_ASSERT(ptr != nullptr);
				_tsz = _sz+sizeof(allocator::word);
			}
			else {
				ptr = env::mem_alloc(_sz);
				ROBO_APP_ASSERT(ptr != nullptr);
				size_t * psz = (((size_t*)ptr) - 1 );
				_tsz = * psz;
			}
		}
		memstat_.total.payload += _sz;
		memstat_.total.size += _tsz;
		memstat_.used.size += _tsz;
		memstat_.total.count++;
		memstat_.used.count++;
		return ptr;
	}


	void system::mem_free_(void* _memo) {
		guard g__;
		size_t sz = allocator::instance_.owned(_memo);		
		size_t _tsz;
		if (sz > 0) {
			allocator::instance_.release(_memo);
			_tsz = sz + sizeof(allocator::word);
		}
		else {
			ROBO_APP_ASSERT(system::env::is_frontend());
			size_t * psz = (((size_t*)_memo) - 1 );
			_tsz = * psz;

			env::mem_free(_memo);
		}
		memstat_.used.size -= _tsz;
		memstat_.used.count--;
	}
	#endif

	void system::printf(cstr _format, va_list _args) {
		#if ROBO_APP_ENV_ENABLED == 1
		string tmp;
		tmp.format(_format, _args);
		env::print(tmp.c_str());
		#endif
	}

	void system::printf(cstr _format, ...) {
		va_list args;
		va_start(args, _format);
		printf(_format, args);
		va_end(args);
	}

	size_t system::sprintf(char_t* _dst, size_t _max_sz, cstr _format, ...) {
		va_list args;
		va_start(args, _format);
		size_t ret = 0;
		#if ROBO_APP_ENV_ENABLED == 1
		ret = env::sprintf(_dst, _max_sz, _format, args);
		#endif
		va_end(args);
		return ret;
	}

}

void* operator new(size_t size) {
	void* tmp;
	#if ROBO_APP_ALLOC_ENABLED == 1
	tmp = robo::system::mem::alloc(size);
	#else
	tmp = malloc(size);
	#endif
	ROBO_APP_ASSERT(tmp != nullptr)
		return tmp;
}

void operator delete(void* ptr) {
	#if ROBO_APP_ALLOC_ENABLED == 1
	robo::system::mem::free(ptr);
	#else
	free(ptr);
	#endif

}


#endif

