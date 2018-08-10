
cmake_minimum_required(VERSION 2.8)
project(linux_timer_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# executeable output path
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# libs output path
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/lib)

add_compile_options(-W)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

IF (CMAKE_BUILD_TYPE MATCHES "Release"   
        OR CMAKE_BUILD_TYPE MATCHES "")  
    MESSAGE(STATUS "CMAKE_BUILD_TYPE is Release")
    SET(CMAKE_BUILD_TYPE "Release")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "Debug")  
    MESSAGE(STATUS "CMAKE_BUILD_TYPE is Debug")
    SET(CMAKE_BUILD_TYPE "Debug")
ELSE ()
    MESSAGE(STATUS "unknown CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE})  
ENDIF ()



add_executable(rtc_test src/rtc_test.c)
target_link_libraries(rtc_test -lrt)

add_executable(timer_test src/timer_test.c)
target_link_libraries(timer_test -lrt)

add_executable(select_test src/select_test.c)

add_executable(setitimer_test src/setitimer_test.c)

add_executable(timerfd_test src/timerfd_test.c)



