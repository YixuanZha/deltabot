cmake_minimum_required(VERSION 3.10)
project(deltabot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

find_package(OpenCV REQUIRED)
message(STATUS "DEBUG: OpenCV Include Dirs are: ${OpenCV_INCLUDE_DIRS}")

find_package(Threads REQUIRED)
find_package(OpenCL REQUIRED) 

set(COMMON_SOURCES
    src/Pwm.cpp
    src/ServoMotorSetting.cpp
    src/DeltaBot.cpp
    src/CaptureCameraFeed.cpp
    src/LineFollower.cpp
    src/CameraFPSTracker.cpp
)

set(CLDL_SOURCES
    CLDL/lib/bpThread.cpp
    CLDL/lib/Layer.cpp
    CLDL/lib/Net.cpp
    CLDL/lib/Neuron.cpp
)

set(KERNEL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CLDL/lib/cldl_kernels.cl)

if(NOT EXISTS ${KERNEL_FILE})
    message(FATAL_ERROR "Kernel file not found at ${KERNEL_FILE}")
endif()

file(COPY ${KERNEL_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(TestServoMotor 
    Tests/TestServoMotor.cpp
    ${COMMON_SOURCES}
    ${CLDL_SOURCES}
)

add_executable(TestSpeedChange 
    Tests/TestSpeedChange.cpp
    ${COMMON_SOURCES}
    ${CLDL_SOURCES}
)

add_executable(TestBotMotion
    Tests/TestBotMotion.cpp
    ${COMMON_SOURCES}
    ${CLDL_SOURCES}
)

add_executable(TestLineFollowing
    Tests/TestLineFollowing.cpp
    ${COMMON_SOURCES}
    ${CLDL_SOURCES}
)

foreach(target TestServoMotor TestSpeedChange TestBotMotion TestLineFollowing)
    target_include_directories(${target} PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/CLDL/include/cldl
        {OpenCL_INCLUDE_DIRS}
    )
    target_link_libraries(${target} PRIVATE 
        ${OpenCV_LIBS}
        Threads::Threads
        OpenCL::OpenCL
    )
endforeach()

install(TARGETS TestServoMotor TestSpeedChange TestBotMotion TestLineFollowing RUNTIME DESTINATION bin)
