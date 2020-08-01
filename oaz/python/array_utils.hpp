#ifndef __ARRAY_UTILS_HPP__
#define __ARRAY_UTILS_HPP__

#include <boost/python/numpy.hpp>
#include "oaz/array/array.hpp"

using namespace oaz::array;

namespace oaz::python {
	
	template <size_t... Is>
	struct Strides;
	
	template <>
	struct Strides<> {
	};

	template <size_t... Is>
	struct StridesGenerator;

	template <size_t I0, size_t... Is>
	struct Strides<I0, Is...> {

		static boost::python::tuple make_python_tuple() {
			return boost::python::make_tuple(I0, Is...);
		}

		template <size_t J>
		using Prepend = Strides< J, I0, Is... >;

		static constexpr size_t First =  I0;
	};
	
	template <size_t I> 
	struct StridesGenerator<I> {
		using Result = Strides< sizeof(float) >;
	};

	
	template <size_t I0, size_t I1, size_t... Is> 
	struct StridesGenerator<I0, I1, Is...> {
		using PreviousResult = typename StridesGenerator<I1, Is...> :: Result;
		using Result = typename PreviousResult::template Prepend<I1 * PreviousResult::First>;
	};

	template<size_t... dimensions>
	boost::python::numpy::ndarray ToNumpy(Array<dimensions...>& array) {
		return boost::python::numpy::from_data(
			array.origin(),
			boost::python::numpy::dtype::get_builtin<float>(),
			boost::python::make_tuple(dimensions...),
			StridesGenerator<dimensions...>::Result::make_python_tuple(),
			boost::python::object()
		);
	}
}

#endif
