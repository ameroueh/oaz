cmake_minimum_required (VERSION 3.13)
project (oaz)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address")

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
find_package(HDF5 REQUIRED COMPONENTS C CXX)

##############################
# Flags

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=${TensorFlow_ABI}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=${TensorFlow_ABI}")

TensorFlow_REQUIRE_C_LIBRARY()
TensorFlow_REQUIRE_SOURCE()

##############################
# Define tests
add_custom_target(
	generate_example_model
	COMMAND 
	python3 ${PROJECT_SOURCE_DIR}/data/generate_connect_four_model.py ${PROJECT_BINARY_DIR}/examples
)

add_custom_target(
	generate_az_search_model
	COMMAND 
	python3 ${PROJECT_SOURCE_DIR}/data/generate_connect_four_model.py ${PROJECT_BINARY_DIR}/test/az/
)

python_add_module(az_connect_four oaz/python/game.cpp)
target_include_directories(
	az_connect_four
	PUBLIC
	${PROJECT_SOURCE_DIR} 
	${PYTHON_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
)
target_link_libraries(
	az_connect_four
	${Boost_LIBRARIES}
	${PYTHON_LIBRARIES}
	TensorFlow_DEP
)
set_target_properties(az_connect_four PROPERTIES PREFIX "")
set_target_properties(az_connect_four PROPERTIES LINKER_LANGUAGE C)

add_executable(connect_four_example examples/connect_four_example.cpp)
target_include_directories(
	connect_four_example 
	PRIVATE 
	${PROJECT_SOURCE_DIR}
	${Boost_INCLUDE_DIRS}
)

set_target_properties(connect_four_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/examples")

add_executable(connect_four_nn_example examples/connect_four_nn_example.cpp)
target_include_directories(
	connect_four_nn_example 
	PRIVATE 
	${PROJECT_SOURCE_DIR}
	${Boost_INCLUDE_DIRS}
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
)
set_target_properties(connect_four_nn_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/examples")

add_dependencies(connect_four_nn_example generate_example_model)

# add_executable(connect_four_self_play_example examples/connect_four_self_play_example.cpp)
# target_include_directories(
# 	connect_four_self_play_example 
# 	PRIVATE 
# 	${PROJECT_SOURCE_DIR}
# 	${Boost_INCLUDE_DIRS}
# 	TensorFlow_DEP 
# 	${TensorFlow_SOURCE_DIR} 
# 	${TensorFlow_BAZEL_GENFILES}
# )
# set_target_properties(connect_four_self_play_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/examples")

# add_dependencies(connect_four_self_play_example generate_example_model)

# add_executable(board_test test/board/board_test.cpp)
# target_include_directories(board_test PRIVATE ${PROJECT_SOURCE_DIR})
# target_include_directories(board_test PRIVATE ${Boost_INCLUDE_DIRS})
# set_target_properties(board_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/board")

add_executable(connect_four_test test/connect_four/connect_four_test.cpp)
target_include_directories(connect_four_test PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(connect_four_test PRIVATE ${Boost_INCLUDE_DIRS})
set_target_properties(connect_four_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/connect_four")

add_executable(tic_tac_toe_test test/tic_tac_toe/tic_tac_toe_test.cpp)
target_include_directories(tic_tac_toe_test PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(tic_tac_toe_test PRIVATE ${Boost_INCLUDE_DIRS})
set_target_properties(tic_tac_toe_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/tic_tac_toe")

add_executable(mcts_test test/mcts/mcts_test.cpp)
target_include_directories(mcts_test PRIVATE ${PROJECT_SOURCE_DIR})
set_target_properties(mcts_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/mcts")

add_executable(hdf5_test test/hdf5/hdf5_test.cpp)
target_include_directories(hdf5_test PRIVATE ${PROJECT_SOURCE_DIR} ${HDF5_INCLUDE_DIRS})
set_target_properties(hdf5_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/hdf5")

# message(STATUS "HDF5_INCLUDE_DIRS=${HDF5_INCLUDE_DIRS}")

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

add_executable(az_search_pool_test test/az/az_search_pool_test.cpp)
target_include_directories(
	az_search_pool_test 
	PRIVATE 
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
)

set_target_properties(
	az_search_pool_test 
	PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/az"
)

add_dependencies(az_search_pool_test generate_az_search_model)

add_executable(self_play_test test/az/self_play_test.cpp)
target_include_directories(
	self_play_test 
	PRIVATE 
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
	${HDF5_INCLUDE_DIRS}
)

set_target_properties(
	self_play_test 
	PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/az"
)

add_dependencies(self_play_test generate_az_search_model)

# add_executable(az_training_test test/az/az_training_test.cpp)
# target_include_directories(
# 	az_training_test 
# 	PRIVATE 
# 	${PROJECT_SOURCE_DIR} 
# 	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
# 	TensorFlow_DEP 
# 	${TensorFlow_SOURCE_DIR} 
# 	${TensorFlow_BAZEL_GENFILES}
# 	${HDF5_INCLUDE_DIRS}

# )

# set_target_properties(
# 	az_training_test 
# 	PROPERTIES 
# 	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/az"
# )

# add_dependencies(az_training_test generate_az_search_model)

add_executable(az_self_play bin/az_self_play.cpp)
target_include_directories(
	az_self_play 
	PRIVATE 
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
	${HDF5_INCLUDE_DIRS}
)
set_target_properties(az_self_play PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

add_executable(az_self_play_tic_tac_toe bin/az_self_play_tic_tac_toe.cpp)
target_include_directories(
	az_self_play_tic_tac_toe 
	PRIVATE 
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
	${HDF5_INCLUDE_DIRS}
)
set_target_properties(az_self_play_tic_tac_toe PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

add_executable(az_self_play_connect_four bin/az_self_play_connect_four.cpp)
target_include_directories(
	az_self_play_connect_four 
	PRIVATE 
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
	${HDF5_INCLUDE_DIRS}
)
set_target_properties(az_self_play_connect_four PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

add_executable(az_evaluate bin/az_evaluate.cpp)
target_include_directories(
	az_evaluate 
	PRIVATE 
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
	${HDF5_INCLUDE_DIRS}
)
set_target_properties(az_evaluate PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

add_executable(az_evaluate_tic_tac_toe bin/az_evaluate_tic_tac_toe.cpp)
target_include_directories(
	az_evaluate_tic_tac_toe 
	PRIVATE 
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
	${HDF5_INCLUDE_DIRS}
)
set_target_properties(az_evaluate_tic_tac_toe PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

add_executable(az_evaluate_connect_four bin/az_evaluate_connect_four.cpp)
target_include_directories(
	az_evaluate_connect_four 
	PRIVATE 
	${PROJECT_SOURCE_DIR} 
	${Boost_INCLUDE_DIRS} # + Tensorflow stuff?
	TensorFlow_DEP 
	${TensorFlow_SOURCE_DIR} 
	${TensorFlow_BAZEL_GENFILES}
	${HDF5_INCLUDE_DIRS}
)
set_target_properties(az_evaluate_connect_four PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

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

add_executable(random_evaluator_test test/random/random_evaluator_test.cpp)
target_include_directories(
	random_evaluator_test 
	PRIVATE 
	${PROJECT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}
)
set_target_properties(
	random_evaluator_test 
	PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/random"
)

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

# add_executable(connect_four_tensorflow_test test/connect_four/connect_four_tensorflow_test.cpp)
# target_include_directories(
# 	connect_four_tensorflow_test 
# 	PRIVATE 
# 	${PROJECT_SOURCE_DIR}
# 	${Boost_INCLUDE_DIRS} 
# 	TensorFlow_DEP 
# 	${TensorFlow_SOURCE_DIR} 
# 	${TensorFlow_BAZEL_GENFILES}
# )
# set_target_properties(connect_four_tensorflow_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/connect_four")


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

# add_executable(nn_trainer_test test/neural_network/nn_trainer_test.cpp)
# target_include_directories(
# 	nn_trainer_test 
# 	PRIVATE 
# 	${Boost_INCLUDE_DIRS}
# 	${PROJECT_SOURCE_DIR} 
# 	TensorFlow_DEP 
# 	${TensorFlow_SOURCE_DIR} 
# 	${TensorFlow_BAZEL_GENFILES}
# )
# set_target_properties(nn_trainer_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/test/neural_network")
# add_dependencies(nn_trainer_test generate_trainer_test_data)

##############################
target_link_libraries(connect_four_example nlohmann_json::nlohmann_json Threads::Threads)

target_link_libraries(connect_four_nn_example nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP)

# target_link_libraries(connect_four_self_play_example nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP)

# add_dependencies(board_test gtest)
# add_dependencies(board_test gmock)
# target_link_libraries(board_test gtest_main gmock_main)

add_dependencies(hdf5_test gtest)
add_dependencies(hdf5_test gmock)
target_link_libraries(hdf5_test gtest_main gmock_main ${HDF5_CXX_LIBRARIES})

add_dependencies(connect_four_test gtest)
add_dependencies(connect_four_test gmock)
target_link_libraries(connect_four_test gtest_main gmock_main)

add_dependencies(tic_tac_toe_test gtest)
add_dependencies(tic_tac_toe_test gmock)
target_link_libraries(tic_tac_toe_test gtest_main gmock_main)

# add_dependencies(connect_four_tensorflow_test gtest)
# add_dependencies(connect_four_tensorflow_test gmock)
# target_link_libraries(connect_four_tensorflow_test gtest_main gmock_main TensorFlow_DEP)

add_dependencies(mcts_test gtest)
add_dependencies(mcts_test gmock)
target_link_libraries(mcts_test gtest_main gmock_main)

add_dependencies(random_evaluator_test gtest)
add_dependencies(random_evaluator_test gmock)
target_link_libraries(random_evaluator_test gtest_main gmock_main Threads::Threads)

add_dependencies(mcts_search_test gtest)
add_dependencies(mcts_search_test gmock)
target_link_libraries(mcts_search_test gtest_main gmock_main nlohmann_json::nlohmann_json Threads::Threads)

add_dependencies(az_search_test gtest)
add_dependencies(az_search_test gmock)
target_link_libraries(az_search_test gtest_main gmock_main nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP)

add_dependencies(az_search_pool_test gtest)
add_dependencies(az_search_pool_test gmock)
target_link_libraries(az_search_pool_test gtest_main gmock_main nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP)

add_dependencies(self_play_test gtest)
add_dependencies(self_play_test gmock)
target_link_libraries(self_play_test gtest_main gmock_main nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP ${HDF5_CXX_LIBRARIES})

# add_dependencies(az_training_test gtest)
# add_dependencies(az_training_test gmock)
# target_link_libraries(az_training_test gtest_main gmock_main nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP ${HDF5_CXX_LIBRARIES})

target_link_libraries(az_self_play nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP ${HDF5_CXX_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(az_evaluate nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP ${HDF5_CXX_LIBRARIES} ${Boost_LIBRARIES})

target_link_libraries(az_self_play_tic_tac_toe nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP ${HDF5_CXX_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(az_evaluate_tic_tac_toe nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP ${HDF5_CXX_LIBRARIES} ${Boost_LIBRARIES})

target_link_libraries(az_self_play_connect_four nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP ${HDF5_CXX_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(az_evaluate_connect_four nlohmann_json::nlohmann_json Threads::Threads TensorFlow_DEP ${HDF5_CXX_LIBRARIES} ${Boost_LIBRARIES})

add_dependencies(mcts_connect_four_test gtest)
add_dependencies(mcts_connect_four_test gmock)
target_link_libraries(mcts_connect_four_test gtest_main gmock_main nlohmann_json::nlohmann_json)

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

# add_dependencies(nn_trainer_test gtest)
# add_dependencies(nn_trainer_test gmock)
# target_link_libraries(nn_trainer_test gtest_main gmock_main TensorFlow_DEP nlohmann_json::nlohmann_json Threads::Threads)
#############################
# Make test runnable with
# $ make test
enable_testing()

# add_test(
# 	NAME board_test
# 	COMMAND board_test
# )

add_test(
	NAME connect_four_test
	COMMAND connect_four_test
)

add_test(
	NAME tic_tac_toe_test
	COMMAND tic_tac_toe_test
)

# add_test(
# 	NAME connect_four_tensorflow_test
# 	COMMAND connect_four_tensorflow_test
# )

add_test(
	NAME mcts_test
	COMMAND mcts_test
)

add_test(
	NAME random_evaluator_test
	COMMAND random_evaluator_test
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
	NAME az_search_pool_test
	COMMAND az_search_pool_test
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test/az
)

add_test(
	NAME self_play_test
	COMMAND self_play_test
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test/az
)

# add_test(
# 	NAME az_training_test
# 	COMMAND az_training_test
# )

add_test(
	NAME mcts_connect_four_test
	COMMAND mcts_connect_four_test
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

# add_test (
# 	NAME nn_trainer_test
# 	COMMAND nn_trainer_test
# 	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test/neural_network
# )

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
