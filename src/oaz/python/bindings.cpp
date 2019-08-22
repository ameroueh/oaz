#include <boost/python.hpp>

#include "hello.hpp"

using namespace boost::python;


BOOST_PYTHON_MODULE(py_oaz) {
	Py_Initialize();
	def("hello", hello);
}
