cmake_minimum_required(VERSION 3.19)
project(oaz)

# DEPENDENCIES

add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")
set(JSON_BuildTests
    OFF
    CACHE INTERNAL "")
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/json" "extern/json")
# add_subdiectory("${PROJECT_SOURCE_DIR}/extern/spdlog" "extern/spdlog")

# CMAKE MODULES

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# EXTERNAL DEPENDENCIES

execute_process(
  COMMAND python -c "from distutils.sysconfig import get_python_inc; \
	print(get_python_inc(), end='')" OUTPUT_VARIABLE PYTHON_INCLUDE_DIR)
execute_process(
  COMMAND
    python -c "import distutils.sysconfig as sysconfig; \
	import os; \
	print( \
		os.path.join(sysconfig.get_config_var('LIBDIR'), \
		sysconfig.get_config_var('LDLIBRARY')), \
	end='')"
  OUTPUT_VARIABLE PYTHON_LIBRARY)
find_package(PythonInterp 3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)
set(Python3_FIND_VIRTUALENV "ONLY")
set(Python3_FIND_STRATEGY "LOCATION")
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(PythonLibs 3 REQUIRED)
set(BOOST_PYTHON_VERSION_SUFFIX
    "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")

find_package(
  Boost 1.69.0 REQUIRED COMPONENTS python${BOOST_PYTHON_VERSION_SUFFIX}
                                   numpy${BOOST_PYTHON_VERSION_SUFFIX})
find_package(SWIG REQUIRED COMPONENTS python)
# execute_process( COMMAND python -m pybind11 --includes COMMAND sed -e
# "s#-I##g" COMMAND sed -e "s# #;#g" OUTPUT_VARIABLE pybind11_INCLUDE_DIRS )
# set(pybind11_INCLUDE_DIRS "") message("pybind11_INCLUDE_DIRS
# ${pybind11_INCLUDE_DIRS}")
message("PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS}")

set(TensorFlow_BUILD_DIR $ENV{TENSORFLOW_LIB})
set(TensorFlow_SOURCE_DIR $ENV{TENSORFLOW_DIR})
set(TensorFlow_BAZEL_GENFILES $ENV{TENSORFLOW_DIR}/bazel-genfiles)
find_package(TensorFlow REQUIRED)
tensorflow_require_c_library()
tensorflow_require_source()

# FLAGS

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=${TensorFlow_ABI}")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=${TensorFlow_ABI}")

# INTERFACE LIBRARIES

add_library(oaz_base INTERFACE)
target_include_directories(oaz_base INTERFACE ${PROJECT_SOURCE_DIR}
                                              ${Boost_INCLUDE_DIRS})

add_library(oaz_test INTERFACE)
add_dependencies(oaz_test gtest gmock)
target_link_libraries(oaz_test INTERFACE gtest_main gmock_main
                                         nlohmann_json::nlohmann_json)

add_library(oaz_python_module INTERFACE)
target_include_directories(oaz_python_module SYSTEM
                           INTERFACE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(oaz_python_module INTERFACE oaz_base ${PYTHON_LIBRARIES}
                                                  ${Boost_LIBRARIES})

add_library(tensorflow INTERFACE)
target_include_directories(
  tensorflow SYSTEM
  INTERFACE ${TensorFlow_SOURCE_DIR} ${TensorFlow_INCLUDE_DIR}
            ${TensorFlow_BAZEL_GENFILES})
target_link_libraries(
  tensorflow INTERFACE ${LINKER_ALLOW_MULDEFS} ${LINKER_WHOLE_ARCHIVE}
                       ${TensorFlow_LIBRARY} ${LINKER_NO_WHOLE_ARCHIVE})

add_library(tensorflow_with_cc_library INTERFACE)
target_link_libraries(
  tensorflow_with_cc_library
  INTERFACE ${LINKER_ALLOW_MULDEFS} ${LINKER_WHOLE_ARCHIVE}
            ${TensorFlow_C_LIBRARY} ${LINKER_NO_WHOLE_ARCHIVE})
target_link_libraries(tensorflow_with_cc_library INTERFACE tensorflow)

add_library(swig INTERFACE)
target_include_directories(swig INTERFACE ${SWIG_DIR})

add_library(pybind11 INTERFACE)
# target_include_directories(pybind11 SYSTEM PUBLIC ${pybind11_INCLUDE_DIRS})

# PYTHON MODULES

python_add_module(thread_pool oaz/python/thread_pool.cpp)
target_link_libraries(thread_pool oaz_python_module)

python_add_module(evaluator oaz/python/evaluator.cpp)
target_link_libraries(evaluator oaz_python_module)

python_add_module(simulation_evaluator oaz/python/simulation_evaluator.cpp
                  oaz/simulation/simulation_evaluator.cpp)
target_link_libraries(simulation_evaluator oaz_python_module)

python_add_module(selection oaz/python/selection.cpp)
target_link_libraries(selection oaz_python_module)

python_add_module(search oaz/python/search.cpp oaz/mcts/search.cpp)
target_link_libraries(search oaz_python_module)

python_add_module(nn_evaluator oaz/python/nn_evaluator.cpp
                  oaz/neural_network/nn_evaluator.cpp)
target_link_libraries(nn_evaluator oaz_python_module tensorflow swig pybind11)

python_add_module(game oaz/python/game.cpp)
target_link_libraries(game oaz_python_module)

python_add_module(cache oaz/python/cache.cpp)
target_link_libraries(cache oaz_python_module)

python_add_module(simple_cache oaz/python/simple_cache.cpp
                  oaz/cache/simple_cache.hpp)
target_link_libraries(simple_cache oaz_python_module)

python_add_module(connect_four oaz/python/game_derived.cpp
                  oaz/games/connect_four.cpp)
target_link_libraries(connect_four oaz_python_module)
target_compile_definitions(
  connect_four
  PRIVATE -DGAME_CLASS_NAME=ConnectFour
          -DGAME_HEADER=oaz/games/connect_four.hpp -DMODULE_NAME=connect_four)

python_add_module(tic_tac_toe oaz/python/game_derived.cpp
                  oaz/games/tic_tac_toe.cpp)
target_link_libraries(tic_tac_toe oaz_python_module)
target_compile_definitions(
  tic_tac_toe
  PRIVATE -DGAME_CLASS_NAME=TicTacToe -DGAME_HEADER=oaz/games/tic_tac_toe.hpp
          -DMODULE_NAME=tic_tac_toe)

python_add_module(bandits oaz/python/game_derived.cpp oaz/games/bandits.cpp)
target_link_libraries(bandits oaz_python_module)
target_compile_definitions(
  bandits PRIVATE -DGAME_CLASS_NAME=Bandits -DGAME_HEADER=oaz/games/bandits.hpp
                  -DMODULE_NAME=bandits)

add_custom_target(all_python)
add_dependencies(
  all_python
  thread_pool
  evaluator
  nn_evaluator
  simulation_evaluator
  selection
  search
  cache
  simple_cache
  game
  connect_four
  bandits
  tic_tac_toe)

# TESTS

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/test)

add_executable(bitboard_test test/bitboard/bitboard_test.cpp)
target_link_libraries(bitboard_test oaz_base oaz_test)

add_executable(connect_four_test test/connect_four/connect_four_test.cpp
                                 oaz/games/connect_four.cpp oaz/mcts/search.cpp)
target_link_libraries(connect_four_test oaz_base oaz_test)

add_executable(simple_cache_test test/cache/simple_cache_test.cpp
                                 oaz/games/tic_tac_toe.cpp)
target_link_libraries(simple_cache_test oaz_base oaz_test)

add_executable(tic_tac_toe_test test/tic_tac_toe/tic_tac_toe_test.cpp
                                oaz/games/tic_tac_toe.cpp)
target_link_libraries(tic_tac_toe_test oaz_base oaz_test)

add_executable(bandits_test test/bandits/bandits_test.cpp oaz/games/bandits.cpp)
target_link_libraries(bandits_test oaz_base oaz_test)

add_executable(mcts_test test/mcts/mcts_test.cpp)
target_link_libraries(mcts_test oaz_base oaz_test)

add_executable(
  mcts_search_test
  test/mcts/mcts_search_test.cpp oaz/mcts/search.cpp oaz/games/connect_four.cpp
  oaz/mcts/search.cpp oaz/simulation/simulation_evaluator.cpp)
target_link_libraries(mcts_search_test oaz_base oaz_test)

add_executable(
  az_search_test test/az/az_search_test.cpp oaz/games/connect_four.cpp
                 oaz/mcts/search.cpp oaz/neural_network/nn_evaluator.cpp)
target_link_libraries(az_search_test oaz_base oaz_test
                      tensorflow_with_cc_library)
add_dependencies(az_search_test generate_evaluator_test_data)

add_executable(
  mcts_connect_four_test
  test/mcts/mcts_connect_four_test.cpp oaz/games/connect_four.cpp
  oaz/mcts/search.cpp oaz/simulation/simulation_evaluator.cpp)
target_link_libraries(mcts_connect_four_test oaz_base oaz_test)

add_executable(
  simulation_evaluator_test
  test/simulation/simulation_evaluator_test.cpp oaz/games/connect_four.cpp
  oaz/mcts/search.cpp oaz/simulation/simulation_evaluator.cpp)
target_link_libraries(simulation_evaluator_test oaz_base oaz_test)

add_executable(thread_pool_test test/thread_pool/thread_pool_test.cpp)
target_link_libraries(thread_pool_test oaz_base oaz_test)

add_executable(mutex_test test/mutex/mutex_test.cpp)
target_link_libraries(mutex_test oaz_base oaz_test)

add_executable(queue_test test/queue/queue_test.cpp)
target_link_libraries(queue_test oaz_base oaz_test)

add_executable(tensorflow_test test/tensorflow/tensorflow_test.cpp)
target_link_libraries(tensorflow_test tensorflow_with_cc_library oaz_test)

add_executable(tensorflow_eager_test test/tensorflow/tensorflow_eager_test.cpp)
target_link_libraries(tensorflow_eager_test oaz_base tensorflow_with_cc_library
                      oaz_test)

add_executable(json_test test/json/json_test.cpp)
target_link_libraries(json_test oaz_base oaz_test)

add_custom_target(
  generate_evaluator_test_data
  COMMAND
    python3
    ${PROJECT_SOURCE_DIR}/test/neural_network/generate_evaluator_test_data.py
    ${PROJECT_BINARY_DIR}/test)

add_executable(
  nn_evaluator_test
  test/neural_network/nn_evaluator_test.cpp oaz/games/connect_four.cpp
  oaz/neural_network/nn_evaluator.cpp)
target_link_libraries(nn_evaluator_test oaz_base oaz_test
                      tensorflow_with_cc_library)
add_dependencies(nn_evaluator_test generate_evaluator_test_data)

add_custom_target(all_tests)
add_dependencies(
  all_tests
  connect_four_test
  tic_tac_toe_test
  bandits_test
  mcts_test
  simulation_evaluator_test
  mcts_search_test
  az_search_test
  mcts_connect_four_test
  thread_pool_test
  mutex_test
  queue_test
  tensorflow_test
  nn_evaluator_test
  simple_cache_test
  tensorflow_eager_test)

enable_testing()

add_test(NAME bitboard_test COMMAND bitboard_test)
add_test(NAME connect_four_test COMMAND connect_four_test)
add_test(NAME tic_tac_toe_test COMMAND tic_tac_toe_test)
add_test(NAME bandits_test COMMAND bandits_test)
add_test(NAME mcts_test COMMAND mcts_test)
add_test(NAME simulation_evaluator_test COMMAND simulation_evaluator_test)
add_test(NAME mcts_search_test COMMAND mcts_search_test)
add_test(NAME mcts_connect_four_test COMMAND mcts_connect_four_test)
add_test(NAME thread_pool_test COMMAND thread_pool_test)
add_test(NAME mutex_test COMMAND mutex_test)
add_test(NAME queue_test COMMAND queue_test)
add_test(NAME tensorflow_test COMMAND tensorflow_test)
add_test(NAME simple_cache_test COMMAND simple_cache_test)
add_test(
  NAME az_search_test
  COMMAND az_search_test
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
add_test(
  NAME nn_evaluator_test
  COMMAND nn_evaluator_test
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
