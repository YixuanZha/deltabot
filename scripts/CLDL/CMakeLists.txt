cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE "Debug")
endif()

project(cldl VERSION 1.0 LANGUAGES CXX)
include(GNUInstallDirs)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_subdirectory(tests)

set(LIBSRC
  lib/Neuron.cpp
  lib/Net.cpp
  lib/Layer.cpp
        lib/bpThread.cpp)

set(PRIVATEHEADERS
  include/cldl/Neuron.h
  include/cldl/Net.h
  include/cldl/Layer.h
        include/cldl/bpThread.h
        include/cldl/CppThread.h)
  

if(UNIX)
  
  add_library(cldl
    SHARED
    ${LIBSRC}
          include/cldl/CppThread.h include/cldl/bpThread.h lib/bpThread.cpp)
  
  target_include_directories(cldl PRIVATE cldl)
  
  set_target_properties(cldl PROPERTIES
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "${PUBLICHEADERS}"
    PRIVATE_HEADER "${PRIVATEHEADERS}")
  
  install(TARGETS cldl
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cldl)
  
endif(UNIX)

  
add_library(cldl_static
  STATIC
  ${LIBSRC}
  )

find_package(Threads)

target_link_libraries(cldl_static ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(cldl_static PRIVATE cldl)

set_target_properties(cldl_static PROPERTIES
  VERSION ${PROJECT_VERSION}
  PUBLIC_HEADER "${PUBLICHEADERS}"
  PRIVATE_HEADER "${PRIVATEHEADERS}")

install(TARGETS cldl_static
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cldl)


find_package(Doxygen)

if(DOXYGEN_FOUND)
    add_custom_target(docs
                      COMMAND ${DOXYGEN_EXECUTABLE}
                      COMMENT "Generating API documentation with Doxygen"
                      VERBATIM)
endif()
