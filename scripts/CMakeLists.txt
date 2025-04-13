cmake_minimum_required(VERSION 3.1.0)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

project(RPLIDAR_A1_Rock5B LANGUAGES CXX)
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

include_directories(rplidarsdk rplidarsdk/src)

set(LIBSRC
  a1lidarrpi.cpp
  rplidarsdk/rplidar_driver.cpp
  rplidarsdk/arch/linux/net_socket.cpp
  rplidarsdk/arch/linux/timer.cpp
  rplidarsdk/arch/linux/net_serial.cpp
  rplidarsdk/hal/thread.cpp
)

add_library(a1lidarrpi STATIC ${LIBSRC})

set_target_properties(a1lidarrpi PROPERTIES
  POSITION_INDEPENDENT_CODE TRUE
  PUBLIC_HEADER a1lidarrpi.h)

target_link_libraries(a1lidarrpi Threads::Threads rt)

add_executable(printdata printdata.cpp)
target_link_libraries(printdata a1lidarrpi Threads::Threads)

add_executable(printRPM printRPM.cpp)
target_link_libraries(printRPM a1lidarrpi Threads::Threads)

add_executable(pwm pwm.cpp)
target_link_libraries(pwm a1lidarrpi Threads::Threads)
