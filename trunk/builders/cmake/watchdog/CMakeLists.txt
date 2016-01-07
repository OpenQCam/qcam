cmake_minimum_required(VERSION 2.6.3)
PROJECT(watchdog)

#make the list with all the source files
FILE(GLOB_RECURSE SOURCES "${CRTMPSERVER_SOURCES_ROOT}/watchdog/src/*.cpp" )


INCLUDE_DIRECTORIES(
  ${LUA_INCLUDE_PATH}
  ${CRTMPSERVER_SOURCES_ROOT}/common/include
  ${CRTMPSERVER_SOURCES_ROOT}/thelib/include
  ${CRTMPSERVER_SOURCES_ROOT}/hardware/include
  ${CRTMPSERVER_SOURCES_ROOT}/watchdog/include
)

ADD_EXECUTABLE(${PROJECT_NAME}  ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  common
  thelib
  hardware
  ${SOCKET_LIB}
  ${OPENSSL_LIBRARY_PATH}
  ${LUA_LIBRARY_PATH}
)

ADD_PRECOMPILED_HEADER(${PROJECT_NAME} "${CRTMPSERVER_SOURCES_ROOT}/common/include/common.h")

