cmake_minimum_required(VERSION 3.10)
project(libgpiod_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(LIBGPIOD REQUIRED libgpiod>=1.6.0)

# if(NOT LIBGPIOD_FOUND)
#     message(FATAL_ERROR "libgpiod not found!")
# endif()

set(COMMON_SOURCES
    src/Pwm.cpp
    src/ServoMotorSetting.cpp
    src/DeltaBot.cpp
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

foreach(target TestServoMotor TestSpeedChange TestBotMotion)
    target_include_directories(${target} PRIVATE
        ${LIBGPIOD_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_directories(${target} PRIVATE
        ${LIBGPIOD_LIBRARY_DIRS}
    )

    target_link_libraries(${target} PRIVATE 
        ${LIBGPIOD_LIBRARIES}
    )

    target_compile_options(${target} PRIVATE
        ${LIBGPIOD_CFLAGS_OTHER}
    )
endforeach()

install(TARGETS TestServoMotor TestSpeedChange TestBotMotion RUNTIME DESTINATION bin)
