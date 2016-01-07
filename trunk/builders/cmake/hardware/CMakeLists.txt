cmake_minimum_required(VERSION 2.6)
PROJECT(hardware)

#make the list with all the source files
FILE(GLOB_RECURSE CXXSOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/*.cpp" )
FILE(GLOB_RECURSE CSOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/*.c" )

FILE(GLOB AUDIO_CL1830_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/cl1830/audio/*.cpp")

#remove cl1830 related files cmakelist files
foreach (fileitem ${AUDIO_CL1830_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()

FILE(GLOB GPIO_CL1830_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/cl1830/gpio/*.cpp")
#remove cl1830 related files cmakelist files
foreach (fileitem ${GPIO_CL1830_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()

FILE(GLOB NVRAM_CL1830_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/cl1830/nvram/*.cpp")
#remove cl1830 related files cmakelist files
foreach (fileitem ${NVRAM_CL1830_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()


FILE(GLOB AUDIO_MT7620_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/mt7620/audio/*.cpp")

#remove cl1830 related files cmakelist files
foreach (fileitem ${AUDIO_MT7620_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()

FILE(GLOB GPIO_MT7620_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/mt7620/gpio/*.cpp")
#remove cl1830 related files cmakelist files
foreach (fileitem ${GPIO_MT7620_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()

FILE(GLOB NVRAM_MT7620_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/mt7620/nvram/*.cpp")
#remove cl1830 related files cmakelist files
foreach (fileitem ${NVRAM_MT7620_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()

FILE(GLOB AUDIO_MT7621_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/mt7621/audio/*.cpp")

#remove cl1830 related files cmakelist files
foreach (fileitem ${AUDIO_MT7621_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()

FILE(GLOB GPIO_MT7621_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/mt7621/gpio/*.cpp")
#remove cl1830 related files cmakelist files
foreach (fileitem ${GPIO_MT7621_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()

FILE(GLOB NVRAM_MT7621_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/mt7621/nvram/*.cpp")
#remove cl1830 related files cmakelist files
foreach (fileitem ${NVRAM_MT7621_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()




FILE(GLOB AUDIO_MT7620PCM_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/mt7620_alsa/audio/*.cpp")

#remove cl1830 related files cmakelist files
foreach (fileitem ${AUDIO_MT7620PCM_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()

FILE(GLOB GPIO_MT7620PCM_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/mt7620_alsa/gpio/*.cpp")
#remove cl1830 related files cmakelist files
foreach (fileitem ${GPIO_MT7620PCM_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()

FILE(GLOB NVRAM_MT7620PCM_SOURCES "${CRTMPSERVER_SOURCES_ROOT}/hardware/src/platform/mt7620_alsa/nvram/*.cpp")
#remove cl1830 related files cmakelist files
foreach (fileitem ${NVRAM_MT7620PCM_SOURCES})
LIST(REMOVE_ITEM CXXSOURCES ${fileitem})
endforeach()




#prepare the include directories
INCLUDE_DIRECTORIES(
  ${CRTMPSERVER_SOURCES_ROOT}/common/include
  ${CRTMPSERVER_SOURCES_ROOT}/thelib/include
  ${CRTMPSERVER_SOURCES_ROOT}/hardware/include/platform/x86/
  ${CRTMPSERVER_SOURCES_ROOT}/hardware/include/audio/mp3enc
  ${CRTMPSERVER_SOURCES_ROOT}/hardware/include
)

#LINK_DIRECTORIES(
#  /usr/local/lib
#)

#create the library
ADD_LIBRARY(qicctrl SHARED ${CSOURCES})
ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_TYPE} ${CXXSOURCES})
ADD_PRECOMPILED_HEADER(${PROJECT_NAME} "${CRTMPSERVER_SOURCES_ROOT}/common/include/common.h")
IF(${LIB_TYPE} MATCHES "SHARED")
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} qicctrl asound faac ${LUA_LIBRARY_PATH} ${TINYXML_LIBRARY_PATH} ${OPENSSL_LIBRARY_PATH} ${CRYPTO_LIBRARY_PATH})
  #TARGET_LINK_LIBRARIES(${PROJECT_NAME} qicctrl asound ${LUA_LIBRARY_PATH} ${TINYXML_LIBRARY_PATH} ${OPENSSL_LIBRARY_PATH} ${CRYPTO_LIBRARY_PATH})
  INSTALL(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CRTMPSERVER_ROOT})
ENDIF(${LIB_TYPE} MATCHES "SHARED")


