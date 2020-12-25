cmake_minimum_required(VERSION 3.17)
project(SyscallEmu VERSION 0.2.0 LANGUAGES C)
set(CMAKE_C_STANDARD "99")

add_library(syscall32 SHARED)
target_sources(syscall32 PUBLIC "src/syscall_32.c")
target_include_directories(syscall32 PUBLIC "include")
set_target_properties(syscall32 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "lib")

add_executable(call)
add_dependencies(call syscall32)
target_sources(call PUBLIC "src/runner.c")
target_include_directories(call PUBLIC "include")
target_link_libraries(call syscall32)

option(VERBOSE "Enable verbose mode" OFF)
if(VERBOSE)
    add_compile_definitions(VERBOSE)
endif(VERBOSE)

add_executable(sysadapt)
add_dependencies(sysadapt call)
target_sources(sysadapt PUBLIC "src/tracer.c")
target_include_directories(sysadapt PUBLIC "include")