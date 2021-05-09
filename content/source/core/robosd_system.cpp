#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include <algorithm>

#if ROBO_APP_SYSTEM_ENABLED == 1

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


namespace robo {
	class allocator {
		friend class system;
		typedef ROBO_SYSTEM_ALLOCATOR_WORD_TYPE word;
		typedef ROBO_SYSTEM_ALLOCATOR_SIZE_TYPE size ;
		enum { size_ = ROBO_SYSTEM_ALLOCATOR_SIZE, bits_ = ROBO_SYSTEM_ALLOCATOR_WORD_SIZE_BITS };

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

		struct block {
			union {
				struct {
					size prev_offset;
					size size;
				};
				word header;
			} ;
			void * memo(void) { return (void *)(&header + 1); }
		};

		word  memo_ [ size_ + 1 ];
		block* top_ = reinterpret_cast<block *>(memo_);
		static allocator instance_;
		size memo_top_ = 0;
		allocator(void) {
			std::fill_n(memo_, size_ + 1, 0);
		}
		size offset_(size_t _sz) {
			size required = (size)(_sz >> bits_);
			size tmp = required << bits_;
			if (tmp < _sz) {
				required++; //данные не выровнены
			}
			return required + 1;
		}
		void* query(size_t _sz) {

			size offset = offset_(_sz);

			size next_top = memo_top_ + offset;

			statistic_.total.query += _sz;
			statistic_.count.query++;

			if ( size_ >= next_top) {
				block* next_ = reinterpret_cast<block*>(memo_+ next_top);
				next_->prev_offset = offset;
				next_->size = 0;
				top_->size = (size)_sz;
				memo_top_ = next_top;
				statistic_.useful += _sz;
				statistic_.used += offset*sizeof(word);
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

		void release_( block* _block ) {
			size offset = offset_( _block->size );

			statistic_.useful -= _block->size;
			statistic_.used -= offset * sizeof(word);

			_block->size = 0;
		}

		void release(void * _memo) {
			block* tmp = (block*)(((word*)_memo) - 1);

			if ( tmp != top_- top_->prev_offset) {
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
				} while ( tmp->size == 0);
			}
		}

		size_t owned(void* _memo) {
			int sz = reinterpret_cast<word*>(_memo) - memo_;
			if (sz > 0 && sz < size_) {
				block* tmp = (block*)(((word*)_memo) - 1);
				return  tmp->size;
			}
			else {
				return 0;
			}
		}
	};

	allocator allocator::instance_;

	void system::enter_(void) {
		if (state_ == state::enabled) {
			if (os::is_frontend()) {
				context_ = os::enter();
			}
			else {
				if (lock_count_ == 0) {
					os::lock();
				}
				lock_count_++;
				context_ = nullptr;
			}
		}
	}

	void system::leave_(void) {
		if (state_ == state::enabled) {
			if (os::is_frontend()) {
				os::leave(context_);
			}
			else {
				if (lock_count_ > 0) {
					lock_count_--;
					if (lock_count_ == 0) {
						os::unlock();
					}
				}
			}
		}
	}

	system::guard::guard(void) {
		system::instance_.enter_();
	}

	system::guard::~guard(void) {
		system::instance_.leave_();
	}
		
	system::system(void) {
		os::begin();

		state_ = state::enabled;
		void* p = malloc(50);
		free(p);

	}

	system::~system(void) {
		os::finish();
		state_ = state::unknown;
	}

	system::lazzyboy::lazzyboy(void)
		: sleep_us_(system::os::time_us()) {
	}

	system::lazzyboy::~lazzyboy(void) {
	}

	robo::time_us_t	 system::lazzyboy::idle_us(void) {
		return  os::time_us() - sleep_us_;
	}

	system::fall::fall(void) {
		os::fall();
	}

	system::fall::~fall(void) {
		os::comeback();
	}

	void* system::mem_alloc_(size_t _sz) {
		void* ptr;
		{
			guard g__;
			if (os::is_backend()) {
				ptr = allocator::instance_.query(_sz);
			}
			else {
				_sz += sizeof(size_t);
				ptr = os::mem_alloc(_sz);
				*(size_t*)ptr = _sz;
				ptr = ((size_t*)ptr) + 1;
			}
		}
		ROBO_APP_ASSERT(ptr != nullptr);
		memstat_.total.size += _sz;
		memstat_.used.size += _sz;
		memstat_.total.count ++;
		memstat_.used.count ++;
		return ptr;
	}
	

	void system::mem_free_(void* _memo) {
		guard g__;
		size_t sz = allocator::instance_.owned(_memo);
		if (sz > 0) {
			allocator::instance_.release( _memo );
		}
		else {
			_memo = (void*)(((size_t*)_memo) - 1);
			sz = *((size_t*)_memo);
			os::mem_free( _memo );
		}
		memstat_.used.size -= sz;
		memstat_.used.count--;

	}

	system system::instance_;

}
void* operator new(size_t size) {
	return robo::system::mem_alloc(size);
}
void operator delete(void* ptr) {
	robo::system::mem_free(ptr);
}



#endif

