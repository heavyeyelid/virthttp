cmake_minimum_required (VERSION 3.12)
project(HeavyEyelid)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

find_package(Boost 1.66
        REQUIRED
        COMPONENTS system)

if(Boost_FOUND)
    include_directories (${Boost_INCLUDE_DIRS}
            thirdparty/gsl/include
            thirdparty/lift/include
            thirdparty/rapidjson/include
            thirdparty/rapidxml_ns)
    add_compile_options ("-O3")
    add_compile_options ("-mtune=native")
    add_compile_options ("-g")
    add_executable(test
            main.cpp
            http_wrapper.hpp
            utils.hpp
            netinc.hpp
            include/virt_wrap/Connection.hpp
            include/virt_wrap/TypesParam.hpp
            include/virt_wrap/type_ops.hpp
            include/virt_wrap/AdminConnection.hpp
            include/virt_wrap/utility.hpp
            include/virt_wrap/Domain.hpp include/virt_wrap/fwd.hpp include/virt_wrap/impl/Connection.hpp include/virt_wrap/impl/Domain.hpp include/virt_wrap/impl/TypedParams.hpp)
    target_link_libraries(test ${Boost_LIBRARIES} virt pthread )
endif()
