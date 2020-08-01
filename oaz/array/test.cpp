#include "array.hpp"

#include <iostream>


using namespace oaz::array;

template<typename T, T... ints>
void print_sequence(std::integer_sequence<T, ints...> int_seq)
{
	    std::cout << "The sequence of size " << int_seq.size() << ": ";
	        ((std::cout << ints << ' '),...);
		    std::cout << '\n';
}

int main() {
	auto s = StridesGenerator<7, 6, 2>::Result::Sequence();

	print_sequence(s);
}
