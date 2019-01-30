cmake_minimum_required(VERSION 3.12)
project(HeavyEyelid)

set(CMAKE_CXX_STANDARD 17)


find_package(Boost 1.66
        REQUIRED
        COMPONENTS system)

if(Boost_FOUND)
    include_directories (${Boost_INCLUDE_DIRS} thirdparty/gsl/include)
    add_compile_options ("-O3")
    add_compile_options ("-mtune=native")
    add_executable(test main.cpp fail.hpp utils.hpp netinc.hpp listener.hpp mainClass.hpp) # List all files that will be compiled
    #target_compile_options() # `executable_name` PUBLIC "-g"
    target_link_libraries(test boost_system virt pthread ) # `executable_name` ${Boost_LIBRAIRIES} pthred crypto ssl
endif()
