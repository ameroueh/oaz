cmake_minimum_required (VERSION 3.13)
project (oaz)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###############################
# Add GoogleTest
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")

# configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
# execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
#   RESULT_VARIABLE result
#   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
# if(result)
#   message(FATAL_ERROR "CMake step for googletest failed: ${result}")
# endif()
# execute_process(COMMAND ${CMAKE_COMMAND} --build .
#   RESULT_VARIABLE result
#   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
# if(result)
#   message(FATAL_ERROR "Build step for googletest failed: ${result}")
# endif()

# # Prevent overriding the parent project's compiler/linker
# # settings on Windows
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# # Add googletest directly to our build. This defines
# # the gtest and gtest_main targets.
# add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
#                  ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
#                  EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
# if (CMAKE_VERSION VERSION_LESS 2.8.11)
#   include_directories("${gtest_SOURCE_DIR}/include")
# endif()

##############################
# Define libraries
find_package(Boost 1.67.0 REQUIRED COMPONENTS system)

##############################
# Define tests
add_executable(connect_four_test test/connect_four_test.cpp src/oaz/games/connect_four.cpp)
target_include_directories(connect_four_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(connect_four_test PRIVATE ${Boost_INCLUDE_DIRS})

add_executable(mcts_test test/mcts_test.cpp test/mcts_test.cpp)
target_include_directories(mcts_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(mcts_test PRIVATE ${Boost_INCLUDE_DIRS})

add_executable(mutex_test test/mutex_test.cpp test/mutex_test.cpp)
target_include_directories(mutex_test PRIVATE ${PROJECT_SOURCE_DIR}/include)


##############################
add_dependencies(connect_four_test gtest)
add_dependencies(connect_four_test gmock)
target_link_libraries(connect_four_test gtest_main gmock_main)

add_dependencies(mcts_test gtest)
add_dependencies(mcts_test gmock)
target_link_libraries(mcts_test gtest_main gmock_main)

add_dependencies(mutex_test gtest)
add_dependencies(mutex_test gmock)
target_link_libraries(mutex_test gtest_main gmock_main)

#############################
# Make test runnable with
# $ make test
enable_testing()
add_test(
	NAME connect_four_test
	COMMAND connect_four_test
)

add_test(
	NAME mcts_test
	COMMAND mcts_test
)

add_test (
	NAME mutex_test
	COMMAND mutex_test
)

############################
# Python bindings
#find_package(Python COMPONENTS Interpreter Development)
#find_package(PythonLibs ${PYTHON_VERSION_STRING} REQUIRED)
#find_package(Boost 1.70.0 REQUIRED COMPONENTS python${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
#
#add_library(py_oaz SHARED ${PROJECT_SOURCE_DIR}/src/python/bindings.cpp ${PROJECT_SOURCE_DIR}/src/hello.cpp)
#
#target_include_directories(py_oaz PRIVATE ${PROJECT_SOURCE_DIR}/include)
#target_include_directories(py_oaz PRIVATE ${Boost_INCLUDE_DIRS})
#target_include_directories(py_oaz PRIVATE ${Python_INCLUDE_DIRS})
#
#target_link_libraries(py_oaz ${Boost_LIBRARIES} ${Python_LIBRARIES})
#
#set_target_properties(py_oaz PROPERTIES PREFIX "")
#set_target_properties(py_oaz PROPERTIES SUFFIX ".so")
#install(TARGETS py_oaz DESTINATION ${PROJECT_BINARY_DIR})
