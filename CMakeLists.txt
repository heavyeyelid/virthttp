cmake_minimum_required (VERSION 3.13)
project (HeavyEyelid)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

find_package(Boost 1.66
        REQUIRED
        COMPONENTS system)

if(Boost_FOUND)
    include_directories (${Boost_INCLUDE_DIRS} thirdparty/gsl/include)
    add_compile_options ("-O3")
    add_compile_options ("-mtune=native")
    add_compile_options ("-g")
    add_executable(test main.cpp fail.hpp utils.hpp netinc.hpp listener.hpp mainClass.hpp include/virt_wrap/Connection.hpp include/virt_wrap/TypesParam.hpp include/virt_wrap/type_ops.hpp include/virt_wrap/AdminConnection.hpp) # List all files that will be compiled
    target_link_libraries(test ${Boost_LIBRARIES} virt pthread ) # `executable_name` ${Boost_LIBRAIRIES} pthred crypto ssl
endif()
