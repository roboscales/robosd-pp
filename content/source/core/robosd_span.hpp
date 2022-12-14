#ifndef robo_span_hpp
#define robo_span_hpp

#include "core/robosd_common.hpp"

namespace robo {
	template<typename T, size_t N> class span {
	public:
		const T* const memo;
		enum { size = N};
		constexpr span(const T(&_data)[N]) noexcept
			: memo(&_data[0]) {}
		
		constexpr span(const T * _data) noexcept
			: memo(&_data[0]) {}
		
		constexpr span(const span & _src) noexcept
			: memo(_src.memo) {}

		constexpr span(const std::initializer_list<T> _src) noexcept
			: memo(_src.begin()) {
			ROBO_APP_ASSERT(N <= _src.size())
		}
		
		template <size_t _offset, size_t _count>
	    constexpr auto subspan() const noexcept
        ->span<T, _count>
		{
			static_assert(_offset + _count <= N, "Offset out of range in span::subspan()");
			return span < T, _count>(memo + _offset);
		}
		constexpr const T & operator[](size_t _idx) const noexcept {
			//(_idx < N, "span<T,N>[] index out of bounds");
			return memo[_idx];
		}
		
	};
	template<class _Tp, size_t _Sz>
		span(const _Tp(&)[_Sz]) -> span<_Tp, _Sz>;
}

#endif
