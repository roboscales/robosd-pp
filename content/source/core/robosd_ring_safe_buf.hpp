#ifndef __robosd_ring_safe_buf_hpp
#define __robosd_ring_safe_buf_hpp
#include  "core/robosd_common.hpp"
namespace robo{
	// Шаблон кольцевого буфера
	// принимает два параметра:
	// размер буфера - должен быть степенью двойки,
	// тип элементов хранящихся в буфере, по умолчанию unsigned char
	template< class G, uint8_t BITS, class DATA_T = uint8_t> class ROBO_EXPORT safe_ring_t
	{
	public:
		// определяем псевдоним для индексов
		static const size_t SIZE = (1 << BITS);
	private:
		// память под буфер
		int test;

		DATA_T _data[SIZE];
		// количество чтений
		size_t _readCount;
		// количество записей
		size_t _writeCount;
		// маска для индексов
		static const size_t _mask = SIZE - 1;
		static const size_t _masksz = (1 << (BITS + 1)) - 1;
		// запись в буфер
		void put_(DATA_T value)
		{
			_data[_writeCount & _mask] = value;
			_writeCount++;
		}
		// чтение из буфера, возвращает текущий символ
		DATA_T get_(void)
		{
			return _data[_readCount++ & _mask];
		}

		// пуст ли буфер
		bool available_(void)const
		{
			return ((_writeCount - _readCount) & _masksz)  != 0;
		}
		// полон ли буфер
		bool full_(void)const
		{
			return ((size_t)(_writeCount - _readCount) & (size_t)~(_mask)) != 0;
		}
		// количество элементов в буфере
		size_t count_(void)const
		{
			return (_writeCount - _readCount) & _masksz;
		}
		size_t space_(void)const
		{
			return SIZE - count_();
		}
		size_t size(void)const
		{
			return SIZE;
		}
		// очистить буфер
		void clear_(void)
		{
			_readCount = 0;
			_writeCount = 0;
		}
	public:
		safe_ring_t(){
			clear_();
			test = 1;
		}
		virtual ~safe_ring_t(){
			clear_();
		}
		bool put(DATA_T * _data, size_t _count){
			G g__;
			if (SIZE - count() >= _count){
				while (_count--)
					put_(*_data++);
				return true;
			}
			else {
				return false;
			}
		}

		size_t get(DATA_T * _data, size_t _max_count){
			G g__;
			size_t _count = count();
			if (_max_count < _count) _count = _max_count;
			size_t sz = _count;
			while (sz--){
				*_data++ = get_();
			}
			return _count;
		}

		// пуст ли буфер
		bool available(void)const
		{
			G g__;
			return available_();
		}
		
		// полон ли буфер
		bool full(void)const
		{
			G g__;
			return full_();
		}
		// количество элементов в буфере
		size_t count(void)const
		{
			G g__;
			return count_();
		}
		size_t space(void)const
		{
			G g__;
			return space_();
		}
		
		bool try_put(DATA_T  _data){
			G g__;
			if (space_() >= 0){
				put_(_data);
				return true;
			}
			else {
				return false;
			}
		}

		bool try_get(DATA_T * _data){
			G g__;
			if( available_() ){
				*_data = get_();
				return true;
			} else {
				return false;
			}
		}
	};
	
}
#endif
