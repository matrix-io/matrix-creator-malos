cmake_minimum_required(VERSION 2.8)

add_definitions(-std=c++11)

include (../cmake/FatalWarnings.cmake)
ADM_EXTRA_WARNINGS()

find_package(Protobuf 3 REQUIRED)
find_package(Threads REQUIRED)

find_library (FFTW_LIBRARIES NAMES fftw3f )

# This won't find ZQM in custom locations.
# FIXME(nelson.castillo): Improve.
FIND_LIBRARY(ZMQ_LIB NAMES zmq)
FIND_LIBRARY(HAL_LIB NAMES matrix_creator_hal)
FIND_LIBRARY(MATRIX_LIB NAMES matrix_malos)
FIND_LIBRARY(MATRIX_ZMQ_LIB NAMES matrix_malos_zmq)
FIND_LIBRARY(WIRINGPI_LIB NAMES wiringPi)


message(STATUS "ZMQ found => " "${ZMQ_LIB}")
message(STATUS "HAL found => " "${HAL_LIB}")
message(STATUS "MATRIX LIB found => " "${MATRIX_LIB}")
message(STATUS "MATRIX ZMQ LIB found => " "${MATRIX_ZMQ_LIB}")

PROTOBUF_GENERATE_CPP(MALOS_PROTO_SRC MALOS_PROTO_HEADER ../protocol-buffers/malos/driver.proto)

include_directories(
  ${PROJECT_BINARY_DIR} # Protos.
  ${PROTOBUF_INCLUDE_DIRS})

set(malos_src
  malos.cpp
  malos_wishbone_base.cpp malos_wishbone_base.h
  driver_imu.cpp driver_imu.h
  driver_humidity.cpp driver_humidity.h
  driver_pressure.cpp driver_pressure.h
  driver_uv.cpp driver_uv.h
  driver_everloop.cpp driver_everloop.h
  driver_micarray_alsa.cpp driver_micarray_alsa.h
  driver_lirc.cpp driver_lirc.h
  driver_servo.cpp driver_servo.h
  driver_gpio.cpp driver_gpio.h
  ${MALOS_PROTO_SRC} ${MALOS_PROTO_HEADER}
)

add_executable(malos ${malos_src})
  set_property(TARGET malos PROPERTY CXX_STANDARD 11)
  target_link_libraries(malos ${PROTOBUF_LIBRARY})
  target_link_libraries(malos ${CMAKE_THREAD_LIBS_INIT})
  target_link_libraries(malos ${HAL_LIB}) 
  target_link_libraries(malos ${MATRIX_LIB})
  target_link_libraries(malos ${MATRIX_ZMQ_LIB} ${ZMQ_LIB})
  target_link_libraries(malos ${WIRINGPI_LIB})
  target_link_libraries(malos ${FFTW_LIBRARIES})

# Binary to be installed.
install(TARGETS malos DESTINATION bin)
