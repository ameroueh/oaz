cmake_minimum_required (VERSION 3.13)
project (oaz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address -std=c++17")

###############################
# Add GoogleTest
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/json" "extern/json")

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
	set(BOOST_PYTHONLIB python3)
endif()


find_package(Boost 1.67.0 REQUIRED COMPONENTS system python program_options numpy)
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

##############################
# Define tests
add_custom_target(
	generate_az_search_model
	COMMAND 
	python3 ${PROJECT_SOURCE_DIR}/data/generate_bandits_model.py ${PROJECT_BINARY_DIR}/test/az/
)

python_add_module(pyoaz_connect_four_core oaz/python/core.cpp)
target_include_directories(
	pyoaz_connect_four_core
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${PYTHON_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_INCLUDE_DIR}
	${TensorFlow_BAZEL_GENFILES}
	${SWIG_DIR}
)
add_library(TensorFlow_DEP2 INTERFACE)
target_include_directories(TensorFlow_DEP2 SYSTEM INTERFACE ${TensorFlow_SOURCE_DIR})
target_include_directories(TensorFlow_DEP2 SYSTEM INTERFACE ${TensorFlow_INCLUDE_DIR})
target_link_libraries(TensorFlow_DEP2 INTERFACE ${LINKER_ALLOW_MULDEFS} ${LINKER_WHOLE_ARCHIVE} ${TensorFlow_LIBRARY} ${LINKER_NO_WHOLE_ARCHIVE})
target_link_libraries(
	pyoaz_connect_four_core
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
	TensorFlow_DEP2
)
target_compile_definitions(
	pyoaz_connect_four_core
	PRIVATE
	-DGAME_CLASS_NAME=ConnectFour
	-DGAME_HEADER=oaz/games/connect_four.hpp
	-DMODULE_NAME=pyoaz_connect_four_core
)

python_add_module(pyoaz_tic_tac_toe_core oaz/python/core.cpp)
target_include_directories(
	pyoaz_tic_tac_toe_core
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${PYTHON_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_INCLUDE_DIR}
	${TensorFlow_BAZEL_GENFILES}
	${SWIG_DIR}
)
target_link_libraries(
	pyoaz_tic_tac_toe_core
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
	TensorFlow_DEP2
)
target_compile_definitions(
	pyoaz_tic_tac_toe_core
	PRIVATE
	-DGAME_CLASS_NAME=TicTacToe
	-DGAME_HEADER=oaz/games/tic_tac_toe.hpp
	-DMODULE_NAME=pyoaz_tic_tac_toe_core
)

python_add_module(pyoaz_bandits_core oaz/python/core.cpp)
target_include_directories(
	pyoaz_bandits_core
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${PYTHON_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_INCLUDE_DIR}
	${TensorFlow_BAZEL_GENFILES}
	${SWIG_DIR}
)
target_link_libraries(
	pyoaz_bandits_core
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
	TensorFlow_DEP2
)
target_compile_definitions(
	pyoaz_bandits_core
	PRIVATE
	-DGAME_CLASS_NAME=Bandits
	-DGAME_HEADER=oaz/games/bandits.hpp
	-DMODULE_NAME=pyoaz_bandits_core
)


add_executable(connect_four_test test/connect_four/connect_four_test.cpp)
target_include_directories(connect_four_test PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(connect_four_test PRIVATE ${Boost_INCLUDE_DIRS})
set_target_properties(connect_four_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/connect_four")

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
add_dependencies(connect_four_test gtest)
add_dependencies(connect_four_test gmock)
target_link_libraries(connect_four_test gtest_main gmock_main)

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
target_link_libraries(az_search_test gtest_main gmock_main nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP)

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
