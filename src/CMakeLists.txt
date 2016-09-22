cmake_minimum_required(VERSION 2.8)

add_definitions(-std=c++11)

include (../cmake/FatalWarnings.cmake)
ADM_EXTRA_WARNINGS()

find_package(Protobuf 3 REQUIRED)
find_package(Threads REQUIRED)

# This won't find ZQM in custom locations.
# FIXME(nelson.castillo): Improve.
FIND_LIBRARY(ZMQ_LIB NAMES zmq)
FIND_LIBRARY(HAL_LIB NAMES matrix_creator_hal)

message(STATUS "ZMQ found => " "${ZMQ_LIB}")
message(STATUS "HAL found => " "${HAL_LIB}")

PROTOBUF_GENERATE_CPP(MALOS_PROTO_SRC MALOS_PROTO_HEADER ../protocol-buffers/malos/driver.proto)

include_directories(
  ${PROJECT_BINARY_DIR} # Protos.
  ${PROTOBUF_INCLUDE_DIRS})

set(zmq_wrapper_lib_sources
  zmq_push.h
  zmq_push.cpp
  zmq_pull.h
  zmq_pull.cpp
)

add_library(zmq_wrapper_lib ${zmq_wrapper_lib_sources})
set_property(TARGET zmq_wrapper_lib PROPERTY CXX_STANDARD 11)
target_link_libraries(zmq_wrapper_lib ${ZMQ_LIB} ${CMAKE_THREAD_LIBS_INIT})

set(malos_src
  malos.cpp
  driver_manager.cpp driver_manager.h
  malos_base.cpp malos_base.h
  malos_wishbone_base.cpp malos_wishbone_base.h
  driver_imu.cpp driver_imu.h
  driver_humidity.cpp driver_humidity.h
  driver_pressure.cpp driver_pressure.h
  driver_uv.cpp driver_uv.h
  driver_everloop.cpp driver_everloop.h
  tcp_client.cpp tcp_client.h
  ${MALOS_PROTO_SRC} ${MALOS_PROTO_HEADER}
)

add_executable(malos ${malos_src})
set_property(TARGET malos PROPERTY CXX_STANDARD 11)
target_link_libraries(malos zmq_wrapper_lib)
target_link_libraries(malos ${HAL_LIB} ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

# Binary to be installed.
install(TARGETS malos DESTINATION bin)
