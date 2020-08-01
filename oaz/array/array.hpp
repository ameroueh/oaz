#include "boost/multi_array.hpp"
#include <utility>
#include <tuple>

#include <boost/python/numpy.hpp>

namespace oaz::array {


	template <class none = void>
	constexpr size_t accumulate_multiply() {
		return 1;
	}
	
	template <size_t i0, size_t... args>
	constexpr size_t accumulate_multiply() {
		return i0 * accumulate_multiply<args...>();
	}

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
	

	template <size_t... dimensions>
	class Array : public boost::multi_array<float, sizeof... (dimensions)> {
		public:
			static constexpr std::array<size_t, sizeof... (dimensions)> Dimensions() {
				return {dimensions...};
			}
			
			static constexpr size_t NumDimensions() {
				return sizeof... (dimensions);
			}

			static constexpr size_t NumElements() {
				return accumulate_multiply<dimensions... > ();
			}

			static constexpr size_t SizeBytes() {
				return sizeof(float) * NumElements();
			}

			Array(): boost::multi_array<float, sizeof... (dimensions)> (
				std::array<size_t, sizeof... (dimensions)>({dimensions...})
			) {}
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
