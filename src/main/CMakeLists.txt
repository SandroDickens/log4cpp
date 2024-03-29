project(liblog4cpp)

set(TARGET_NAME log4cpp)

file(GLOB SRC_FILES "*.cpp")

if (CMAKE_HOST_WIN32)
    add_library(${TARGET_NAME} STATIC ${SRC_FILES})
else ()
    add_library(${TARGET_NAME} SHARED ${SRC_FILES})
endif ()

find_package(Boost 1.75 REQUIRED COMPONENTS json)
if (Boost_FOUND)
    #message(STATUS "Boost_LIB_VERSION = ${Boost_VERSION}")
    #message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
    #message(STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
    #message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
endif ()

if (CMAKE_HOST_WIN32)
    target_link_libraries(${TARGET_NAME} ws2_32)
endif ()

if (CMAKE_HOST_UNIX)
    target_link_libraries(${TARGET_NAME} pthread)
    if (ENABLE_ASAN)
        target_link_libraries(${TARGET_NAME} asan)
    endif ()
endif ()
