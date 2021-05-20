#ifndef roboxx_ring_hpp
#define roboxx_ring_hpp
#include <stdint.h>
namespace robo{
	class ring
	{
	private:
		// память под буфер

		uint8_t * data_ = nullptr;
		// количество чтений
		unsigned int readCount_ = 0;
		// количество записей
		unsigned int writeCount_ = 0;
		// маска для индексов
		unsigned int mask_ = 0;
		unsigned int masksz_ = 0;
		uint8_t bits_ = 0;
		unsigned int size_ = 0;
	public:
		unsigned int size(void) { return size_; }
		// запись в буфер
		inline void put(uint8_t value)
		{
			data_[writeCount_ & mask_] = value;
			writeCount_++;
		}
		// чтение из буфера, возвращает текущий символ
		inline uint8_t get()
		{
			return data_[readCount_++ & mask_];
		}

		// пуст ли буфер
		inline bool available()const
		{
			return ((writeCount_ - readCount_) & masksz_)  != 0;
		}
		// полон ли буфер
		inline bool full()const
		{
			return ((unsigned int)(writeCount_ - readCount_) & (unsigned int)~(mask_)) != 0;
		}
		// количество элементов в буфере
		unsigned int count()const
		{
			return (writeCount_ - readCount_) & masksz_;
		}
		unsigned int space()const
		{
			return size_ - count();
		}
		// очистить буфер
		inline void clear()
		{
			readCount_ = 0;
			writeCount_ = 0;
		}
		bool setup(uint8_t _bits) {
			bits_ = _bits;
			size_ = (1 << bits_);
			data_ = new uint8_t[size_];
			readCount_ = 0;
			writeCount_ = 0;
			mask_ = size_ - 1;
			masksz_ = (1 << (bits_ + 1)) - 1;
			return  data_ != nullptr;
		}
		void cleanup(void) {
			if (data_ != nullptr) {
				delete [] data_;
				data_ = nullptr;
				bits_ = 0;
				size_ = 0;
				readCount_ = 0;
				writeCount_ = 0;
				mask_ = 0;
				masksz_ = 0;
			}
		}

		ring(){
		}

		ring(uint8_t _bits) {
			setup(_bits);
		}

		virtual ~ring(){
			cleanup();
		}

		bool put(uint8_t * _data, unsigned int _count){
			if (size_ - count() >= _count){
				while (_count--)
					put(*_data++);
				return true;
			}
			else {
				return false;
			}
		}

		unsigned int get(uint8_t* _data, unsigned int _max_count){
			unsigned int _count = count();
			if (_max_count < _count) _count = _max_count;
			unsigned int sz = _count;
			while (sz--){
				*_data++ = get();
			}
			return _count;
		}

	};
}
#endif
