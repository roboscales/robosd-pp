#ifndef __robosd_ring_buf_h
#define __robosd_ring_buf_h
#include <stdint.h>
// Шаблон кольцевого буфера
// принимает два параметра:
// размер буфера - должен быть степенью двойки,
// тип элементов хранящихся в буфере, по умолчанию unsigned char
template< uint8_t BITS, class DATA_T = uint8_t> class ring_t
{
public:
	// определяем псевдоним для индексов
	static const unsigned SIZE = (1 << BITS);
private:
	// память под буфер
	int test;

	DATA_T _data[SIZE];
	// количество чтений
	unsigned _readCount;
	// количество записей
	unsigned _writeCount;
	// маска для индексов
	static const unsigned _mask = SIZE - 1;
	static const unsigned _masksz = (1 << (BITS + 1)) - 1;
public:
	// запись в буфер
	inline void put(DATA_T value)
	{
		_data[_writeCount & _mask] = value;
		_writeCount++;
	}
	// чтение из буфера, возвращает текущий символ
	inline DATA_T get(void)
	{
		return _data[_readCount++ & _mask];
	}

	// пуст ли буфер
	inline bool available(void)const
	{
		return ((_writeCount - _readCount) & _masksz)  != 0;
	}
	// полон ли буфер
	inline bool full(void)const
	{
		return ((unsigned)(_writeCount - _readCount) & (unsigned)~(_mask)) != 0;
	}
	// количество элементов в буфере
	unsigned count(void)const
	{
		return (_writeCount - _readCount) & _masksz;
	}
	unsigned space(void)const
	{
		return SIZE - count();
	}
	unsigned size(void)const
	{
		return SIZE;
	}
	// очистить буфер
	inline void clear(void)
	{
		_readCount = 0;
		_writeCount = 0;
	}
	ring_t(void){
		clear();
		test = 1;
	}
	virtual ~ring_t(void){
		clear();
	}
	bool put(const DATA_T * _data, unsigned _count){
		if (SIZE - count() >= _count){
			while (_count--)
				put(*_data++);
			return true;
		}
		else {
			return false;
		}
	}

	unsigned get(DATA_T * _data, unsigned __max_count){
		unsigned _count = count();
		if (__max_count < _count) _count = __max_count;
		unsigned sz = _count;
		while (sz--){
			*_data++ = get();
		}
		return _count;
	}

};
#endif
