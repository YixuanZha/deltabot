cmake_minimum_required(VERSION 3.10)
project(deltabot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)
message(STATUS "DEBUG: OpenCV Include Dirs are: ${OpenCV_INCLUDE_DIRS}")

find_package(Threads REQUIRED)

set(COMMON_SOURCES
    src/Pwm.cpp
    src/ServoMotorSetting.cpp
    src/DeltaBot.cpp
    src/CaptureCameraFeed.cpp
)

add_executable(TestCamera
    Tests/TestCamera.cpp
    ${COMMON_SOURCES}
)

add_executable(TestServoMotor 
    Tests/TestServoMotor.cpp
    ${COMMON_SOURCES}
)

add_executable(TestSpeedChange 
    Tests/TestSpeedChange.cpp
    ${COMMON_SOURCES}
)

add_executable(TestBotMotion
    Tests/TestBotMotion.cpp
    ${COMMON_SOURCES}
)

add_executable(TestBotAndCamera
    Tests/TestBotAndCamera.cpp
    ${COMMON_SOURCES}
)

foreach(target TestCamera TestServoMotor TestSpeedChange TestBotMotion TestBotAndCamera)
    target_include_directories(${target} PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${target} PRIVATE 
        ${OpenCV_LIBS}
        Threads::Threads
    )
endforeach()

install(TARGETS TestCamera TestServoMotor TestSpeedChange TestBotMotion TestBotAndCamera RUNTIME DESTINATION bin)
