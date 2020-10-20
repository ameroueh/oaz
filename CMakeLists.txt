cmake_minimum_required (VERSION 3.13)
project (oaz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -std=c++17 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address -std=c++17")

###############################
# Add GoogleTest
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/json" "extern/json")
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/spdlog" "extern/spdlog")


# Add tensorflow-cmake
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

set(TensorFlow_BUILD_DIR ${PROJECT_SOURCE_DIR}/extern/tensorflow/bazel-bin/tensorflow)
set(TensorFlow_SOURCE_DIR ${PROJECT_SOURCE_DIR}/extern/tensorflow)
set(TensorFlow_BAZEL_GENFILES ${PROJECT_SOURCE_DIR}/extern/tensorflow/bazel-genfiles)


##############################
# Define libraries
find_package(PythonLibs 3 REQUIRED)

if(UNIX)
	set(BOOST_PYTHON_LIB python-py36)
else()
	set(BOOST_PYTHON_LIB python3)
endif()


find_package(Boost 1.69.0 REQUIRED COMPONENTS python36 numpy36)
find_package(TensorFlow REQUIRED)
find_package(Threads REQUIRED)
find_package(SWIG REQUIRED COMPONENTS python)

message(STATUS "Swig lib at ${SWIG_DIR}")

##############################
# Flags

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=${TensorFlow_ABI}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=${TensorFlow_ABI}")

TensorFlow_REQUIRE_C_LIBRARY()
TensorFlow_REQUIRE_SOURCE()


### PYTHON MODULES
python_add_module(thread_pool oaz/python/thread_pool.cpp)
target_include_directories(
	thread_pool
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	thread_pool
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
python_add_module(evaluator oaz/python/evaluator.cpp)
target_include_directories(
	evaluator
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	evaluator
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
python_add_module(simulation_evaluator oaz/python/simulation_evaluator.cpp)
target_include_directories(
	simulation_evaluator
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	simulation_evaluator
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
python_add_module(selection oaz/python/selection.cpp)
target_include_directories(
	selection
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	selection
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
python_add_module(search oaz/python/search.cpp)
target_include_directories(
	search
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	search
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
add_library(TensorFlow_DEP2 INTERFACE)
target_include_directories(
	TensorFlow_DEP2 
	SYSTEM 
	INTERFACE 
	${TensorFlow_SOURCE_DIR}
	${TensorFlow_INCLUDE_DIR}
)
# target_include_directories(
# 	TensorFlow_DEP2 SYSTEM INTERFACE ${TensorFlow_INCLUDE_DIR})
target_link_libraries(
	TensorFlow_DEP2 
	INTERFACE 
	${LINKER_ALLOW_MULDEFS} 
	${LINKER_WHOLE_ARCHIVE} 
	${TensorFlow_LIBRARY} 
	${LINKER_NO_WHOLE_ARCHIVE}
)
python_add_module(nn_evaluator oaz/python/nn_evaluator.cpp)
target_include_directories(
	nn_evaluator
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
	${SWIG_DIR}
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_INCLUDE_DIR}
	${TensorFlow_BAZEL_GENFILES}
)
target_link_libraries(
	nn_evaluator
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
	TensorFlow_DEP2
)
python_add_module(game oaz/python/game.cpp)
target_include_directories(
	game
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	game
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
target_link_libraries(
	game
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
python_add_module(connect_four oaz/python/game_derived.cpp)
target_include_directories(
	connect_four
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	connect_four
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
target_compile_definitions(
	connect_four
	PRIVATE
	-DGAME_CLASS_NAME=ConnectFour
	-DGAME_HEADER=oaz/games/connect_four.hpp
	-DMODULE_NAME=connect_four
)
python_add_module(tic_tac_toe oaz/python/game_derived.cpp)
target_include_directories(
	tic_tac_toe
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	tic_tac_toe
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
target_compile_definitions(
	tic_tac_toe
	PRIVATE
	-DGAME_CLASS_NAME=TicTacToe
	-DGAME_HEADER=oaz/games/tic_tac_toe.hpp
	-DMODULE_NAME=tic_tac_toe
)
python_add_module(bandits oaz/python/game_derived.cpp)
target_include_directories(
	bandits
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	bandits
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
target_compile_definitions(
	bandits
	PRIVATE
	-DGAME_CLASS_NAME=Bandits
	-DGAME_HEADER=oaz/games/bandits.hpp
	-DMODULE_NAME=bandits
)
python_add_module(cache oaz/python/cache.cpp)
target_include_directories(
	cache
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	cache
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)
python_add_module(simple_cache oaz/python/simple_cache.cpp)
target_include_directories(
	simple_cache
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(
	simple_cache
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)

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
	tic_tac_toe
)

### TESTS
add_custom_target(
	generate_az_search_model
	COMMAND 
	python3 ${PROJECT_SOURCE_DIR}/data/generate_connect_four_model.py ${PROJECT_BINARY_DIR}/test/az/
)


add_executable(bitboard_test test/bitboard/bitboard_test.cpp)
target_include_directories(bitboard_test PRIVATE ${PROJECT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(bitboard_test ${Boost_LIBRARIES})
set_target_properties(bitboard_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/bitboard")

add_executable(connect_four_test test/connect_four/connect_four_test.cpp)
target_include_directories(connect_four_test PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(connect_four_test PRIVATE ${Boost_INCLUDE_DIRS})
set_target_properties(connect_four_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/connect_four")

add_executable(simple_cache_test test/cache/simple_cache_test.cpp)
target_include_directories(simple_cache_test PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(simple_cache_test PRIVATE ${Boost_INCLUDE_DIRS})
set_target_properties(simple_cache_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/cache")

add_executable(tic_tac_toe_test test/tic_tac_toe/tic_tac_toe_test.cpp)
target_include_directories(tic_tac_toe_test PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(tic_tac_toe_test PRIVATE ${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
set_target_properties(tic_tac_toe_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/tic_tac_toe")

add_executable(bandits_test test/bandits/bandits_test.cpp)
target_include_directories(bandits_test PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(bandits_test PRIVATE ${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
set_target_properties(bandits_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/bandits")

add_executable(mcts_test test/mcts/mcts_test.cpp)
target_include_directories(mcts_test PRIVATE ${PROJECT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
set_target_properties(mcts_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/mcts")

add_executable(mcts_search_test test/mcts/mcts_search_test.cpp)
target_include_directories(
	mcts_search_test 
	PRIVATE 
	${PROJECT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}
)
set_target_properties(
	mcts_search_test 
	PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/mcts"
)

add_executable(az_search_test test/az/az_search_test.cpp)
target_include_directories(
	az_search_test 
	PRIVATE 
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
)

set_target_properties(
	az_search_test 
	PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/az"
)

add_dependencies(az_search_test generate_az_search_model)

add_executable(mcts_connect_four_test test/mcts/mcts_connect_four_test.cpp)

target_include_directories(
	mcts_connect_four_test 
	PRIVATE 
	${PROJECT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}
)
set_target_properties(
	mcts_connect_four_test 
	PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/mcts"
)

add_executable(simulation_evaluator_test test/simulation/simulation_evaluator_test.cpp)
target_include_directories(
	simulation_evaluator_test 
	PRIVATE 
	${PROJECT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}
)
set_target_properties(
	simulation_evaluator_test 
	PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/simulation"
)

add_executable(thread_pool_test test/thread_pool/thread_pool_test.cpp)
target_include_directories(thread_pool_test PRIVATE ${PROJECT_SOURCE_DIR})
set_target_properties(thread_pool_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/thread_pool")

add_executable(mutex_test test/mutex/mutex_test.cpp)
target_include_directories(mutex_test PRIVATE ${PROJECT_SOURCE_DIR})
set_target_properties(mutex_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/mutex")

add_executable(queue_test test/queue/queue_test.cpp) 
target_include_directories(queue_test PRIVATE ${PROJECT_SOURCE_DIR})
set_target_properties(queue_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/queue")

add_executable(tensorflow_test test/tensorflow/tensorflow_test.cpp)
target_include_directories(tensorflow_test PRIVATE TensorFlow_DEP ${TensorFlow_SOURCE_DIR} ${TensorFlow_BAZEL_GENFILES})
set_target_properties(tensorflow_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/tensorflow")

add_executable(json_test test/json/json_test.cpp)
target_include_directories(json_test PRIVATE ${PROJECT_SOURCE_DIR})
set_target_properties(json_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/json")
add_custom_target(
	generate_evaluator_test_data
	COMMAND 
	python3 ${PROJECT_SOURCE_DIR}/test/neural_network/generate_evaluator_test_data.py ${PROJECT_BINARY_DIR}/test/neural_network
)

add_custom_target(
	generate_trainer_test_data
	COMMAND 
	python3 ${PROJECT_SOURCE_DIR}/data/generate_connect_four_model_for_trainer.py ${PROJECT_BINARY_DIR}/test/neural_network
)

add_executable(nn_evaluator_test test/neural_network/nn_evaluator_test.cpp)
target_include_directories(
	nn_evaluator_test 
	PRIVATE 
	${Boost_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR} 
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
)
set_target_properties(nn_evaluator_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/neural_network")
add_dependencies(nn_evaluator_test generate_evaluator_test_data)

##############################
add_dependencies(bitboard_test gtest)
add_dependencies(bitboard_test gmock)
target_link_libraries(bitboard_test gtest_main gmock_main)

add_dependencies(connect_four_test gtest)
add_dependencies(connect_four_test gmock)
target_link_libraries(connect_four_test gtest_main gmock_main)

add_dependencies(simple_cache_test gtest)
add_dependencies(simple_cache_test gmock)
target_link_libraries(
	simple_cache_test 
	gtest_main 
	gmock_main
	Threads::Threads 
)


add_dependencies(tic_tac_toe_test gtest)
add_dependencies(tic_tac_toe_test gmock)
target_link_libraries(
	tic_tac_toe_test 
	gtest_main 
	gmock_main
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)

add_dependencies(bandits_test gtest)
add_dependencies(bandits_test gmock)
target_link_libraries(
	bandits_test 
	gtest_main 
	gmock_main
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
)


add_dependencies(mcts_test gtest)
add_dependencies(mcts_test gmock)
target_link_libraries(mcts_test gtest_main gmock_main)

add_dependencies(simulation_evaluator_test gtest)
add_dependencies(simulation_evaluator_test gmock)
target_link_libraries(simulation_evaluator_test gtest_main gmock_main Threads::Threads)

add_dependencies(mcts_search_test gtest)
add_dependencies(mcts_search_test gmock)
target_link_libraries(mcts_search_test gtest_main gmock_main nlohmann_json::nlohmann_json Threads::Threads)

add_dependencies(az_search_test gtest)
add_dependencies(az_search_test gmock)
target_link_libraries(
	az_search_test 
	gtest_main 
	gmock_main 
	nlohmann_json::nlohmann_json 
	Threads::Threads 
	TensorFlow_DEP
)

add_dependencies(mcts_connect_four_test gtest)
add_dependencies(mcts_connect_four_test gmock)
target_link_libraries(mcts_connect_four_test gtest_main gmock_main nlohmann_json::nlohmann_json)

add_dependencies(thread_pool_test gtest)
add_dependencies(thread_pool_test gmock)
target_link_libraries(thread_pool_test gtest_main gmock_main)


add_dependencies(mutex_test gtest)
add_dependencies(mutex_test gmock)
target_link_libraries(mutex_test gtest_main gmock_main)

add_dependencies(queue_test gtest)
add_dependencies(queue_test gmock)
target_link_libraries(queue_test gtest_main gmock_main)

add_dependencies(tensorflow_test gtest)
add_dependencies(tensorflow_test gmock)
target_link_libraries(tensorflow_test gtest_main gmock_main TensorFlow_DEP)

add_dependencies(json_test gtest)
add_dependencies(json_test gmock)
target_link_libraries(json_test PRIVATE gtest_main gmock_main nlohmann_json::nlohmann_json)

add_dependencies(nn_evaluator_test gtest)
add_dependencies(nn_evaluator_test gmock)
target_link_libraries(nn_evaluator_test gtest_main gmock_main TensorFlow_DEP nlohmann_json::nlohmann_json Threads::Threads)

enable_testing()

add_custom_target(build_tests)
add_dependencies(
	build_tests
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
)

add_test(
	NAME bitboard_test
	COMMAND bitboard_test
)


add_test(
	NAME connect_four_test
	COMMAND connect_four_test
)

add_test(
	NAME tic_tac_toe_test
	COMMAND tic_tac_toe_test
)

add_test(
	NAME bandits_test
	COMMAND bandits_test
)

add_test(
	NAME mcts_test
	COMMAND mcts_test
)

add_test(
	NAME simulation_evaluator_test
	COMMAND simulation_evaluator_test
)

add_test(
	NAME mcts_search_test
	COMMAND mcts_search_test
)

add_test(
	NAME az_search_test
	COMMAND az_search_test
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test/az
)

add_test(
	NAME mcts_connect_four_test
	COMMAND mcts_connect_four_test
)

add_test (
	NAME thread_pool_test
	COMMAND thread_pool_test
)


add_test (
	NAME mutex_test
	COMMAND mutex_test
)

add_test (
	NAME queue_test
	COMMAND queue_test
)

add_test (
	NAME tensorflow_test
	COMMAND tensorflow_test
)

add_test (
	NAME nn_evaluator_test
	COMMAND nn_evaluator_test
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test/neural_network
)

add_test(
	NAME simple_cache_test
	COMMAND simple_cache_test
)

