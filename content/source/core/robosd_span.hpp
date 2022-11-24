#ifndef robo_span_hpp
#define robo_span_hpp

#include "core/robosd_common.hpp"

namespace robo {
	template<typename T, size_t N> class span {
	public:
		constexpr span(const T(&_data)[N]) noexcept
			: data_(&_data[0]) {}
		constexpr span(const T * _data) noexcept
			: data_(&_data[0]) {}
		constexpr span(const span & _src) noexcept
			: data_(_src.data()) {}
		template <size_t _offset, size_t _count>
	    constexpr auto subspan() const noexcept
        ->span<T, _count>
		{
			static_assert(_offset + _count <= N, "Offset out of range in span::subspan()");
			return span < T, _count>(data_+ _offset);
		}
		constexpr const T & operator[](size_t _idx) const noexcept {
			//(_idx < N, "span<T,N>[] index out of bounds");
			return data_[_idx];
		}
		constexpr const T* data(void) const noexcept { return data_; }
		constexpr const size_t size(void) const noexcept { return N; }
		constexpr const T * begin(void) const noexcept { return &data_[0]; }
	private:
		const T *  data_;
	};
	template<class _Tp, size_t _Sz>
		span(const _Tp(&)[_Sz]) -> span<_Tp, _Sz>;
}

#endif
