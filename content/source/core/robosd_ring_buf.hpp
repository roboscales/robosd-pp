#ifndef __robosd_ring_buf_h
#define __robosd_ring_buf_h
#include  "core/robosd_common.hpp"
namespace robo{
	// Шаблон кольцевого буфера
	// принимает два параметра:
	// размер буфера - должен быть степенью двойки,
	// тип элементов хранящихся в буфере, по умолчанию unsigned char
	template< uint8_t BITS, class DATA_T = uint8_t> class ROBO_EXPORT ring_t
	{
	public:
		// определяем псевдоним для индексов
		static const size_t SIZE = (1 << BITS);
	private:
		// память под буфер
		int test;

		DATA_T _data[SIZE];
		// количество чтений
		volatile size_t _readCount;
		// количество записей
		volatile size_t _writeCount;
		// маска для индексов
		static const size_t _mask = SIZE - 1;
		static const size_t _masksz = (1 << (BITS + 1)) - 1;
	public:
		// запись в буфер
		inline void put(DATA_T value)
		{
			_data[_writeCount & _mask] = value;
			_writeCount++;
		}
		// чтение из буфера, возвращает текущий символ
		inline DATA_T get()
		{
			return _data[_readCount++ & _mask];
		}

		// пуст ли буфер
		inline bool available()const
		{
			return ((_writeCount - _readCount) & _masksz)  != 0;
		}
		// полон ли буфер
		inline bool full()const
		{
			return ((size_t)(_writeCount - _readCount) & (size_t)~(_mask)) != 0;
		}
		// количество элементов в буфере
		size_t count()const
		{
			return (_writeCount - _readCount) & _masksz;
		}
		size_t space()const
		{
			return SIZE - count();
		}
		size_t size()const
		{
			return SIZE;
		}
		// очистить буфер
		inline void clear()
		{
			_readCount = 0;
			_writeCount = 0;
		}
		ring_t(){
			clear();
			test = 1;
		}
		virtual ~ring_t(){
			clear();
		}
		bool put(DATA_T * _data, size_t _count){
			if (SIZE - count() >= _count){
				while (_count--)
					put(*_data++);
				return true;
			}
			else {
				return false;
			}
		}

		size_t get(DATA_T * _data, size_t __max_count){
			size_t _count = count();
			if (__max_count < _count) _count = __max_count;
			size_t sz = _count;
			while (sz--){
				*_data++ = get();
			}
			return _count;
		}

	};
}
#endif
