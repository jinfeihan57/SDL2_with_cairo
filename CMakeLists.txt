
cmake_minimum_required(VERSION 3.16.3)
project(hello_world)

enable_testing()

option(BUILD_DEBUG "Builds debug!!!" OFF)
if(BUILD_DEBUG)
    add_compile_options("-g")
endif()

set(HELLO_SDL_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(HELLO_SDL_BIN_ROOT ${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_CXX_FLAGS "-mwindows")
else()
    message(STATUS "OS: " ${CMAKE_SYSTEM_NAME})
endif(CMAKE_SYSTEM_NAME MATCHES "Windows")

set(PKG_CONFIG_ARGN "--static")
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAIRO REQUIRED IMPORTED_TARGET cairo) # PkgConfig::LIBCAIRO
pkg_check_modules(LIBSKIA REQUIRED IMPORTED_TARGET skia) # PkgConfig::LIBCAIRO

message(WARNING "LIBCAIRO_LIBRARIES:" ${LIBCAIRO_LIBRARIES})
message(WARNING "LIBCAIRO_LIBRARY_DIRS:" ${LIBCAIRO_LIBRARY_DIRS})
message(WARNING "LIBCAIRO_INCLUDE_DIRS:" ${LIBCAIRO_INCLUDE_DIRS})
message(WARNING "LIBCAIRO_LDFLAGS:" ${LIBCAIRO_LDFLAGS})
message(WARNING "LIBCAIRO_CFLAGS:" ${LIBCAIRO_CFLAGS})
message(WARNING "LIBCAIRO_CFLAGS_OTHER:" ${LIBCAIRO_CFLAGS_OTHER})

add_executable(main ${CMAKE_CURRENT_SOURCE_DIR}/SDL2_cairo.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd/SDL-release-2.28.3)

target_include_directories(main PRIVATE ${LIBCAIRO_INCLUDE_DIRS} ${LIBSKIA_INCLUDE_DIRS})
target_link_directories(main PRIVATE ${LIBCAIRO_LIBRARY_DIRS} ${LIBSKIA_LIBRARY_DIRS})
target_link_libraries(main PRIVATE SDL2::SDL2-static SDL2::SDL2main
                    -Wl,-Bstatic ${LIBCAIRO_LIBRARIES} -Wl,-Bdynamic
                    -Wl,-Bstatic ${LIBSKIA_LIBRARIES} -Wl,-Bdynamic)

# 设置 编译测试选项
option(BUILD_TEST "Builds test!!!" OFF)
if(BUILD_TEST)
    add_subdirectory(tests)
endif()
