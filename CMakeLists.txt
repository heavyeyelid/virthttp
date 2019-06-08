cmake_minimum_required (VERSION 3.12)
project (HeavyEyelid)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")
if (!MACOS AND !MINGW)
    set (CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

find_package (Boost 1.66
        REQUIRED
        COMPONENTS system)

find_package (LibVirt
        REQUIRED)

if (Boost_FOUND AND LibVirt_FOUND)
    include_directories (${Boost_INCLUDE_DIRS} ${LibVirt_INCLUDE_DIRS}
            thirdparty/gsl/include
            thirdparty/lift/include
            thirdparty/rapidjson/include
            thirdparty/rapidxml_ns
            thirdparty/inih
            thirdparty/ctre/include
            include)

    add_compile_options ("-O1")
    add_compile_options ("-mtune=native")
    add_compile_options ("-g")
    add_executable (HE_debug
            main.cpp
            include/logger.hpp
            include/wrapper/http_wrapper.hpp
            include/utils.hpp
            include/virt_wrap/Connection.hpp
            include/virt_wrap/TypesParam.hpp
            include/virt_wrap/type_ops.hpp
            include/virt_wrap/AdminConnection.hpp
            include/virt_wrap/utility.hpp
            include/virt_wrap/Domain.hpp
            include/virt_wrap/fwd.hpp
            include/virt_wrap/impl/Connection.hpp
            include/virt_wrap/impl/Domain.hpp
            include/virt_wrap/impl/TypedParams.hpp
            include/wrapper/config.hpp
            include/virt_wrap.hpp
            include/logger.hpp
            include/fwd.hpp
            include/urlparser.hpp)

    target_link_libraries (HE_debug ${Boost_LIBRARIES} ${LibVirt_LIBRARIES} pthread )
    if (WIN32)
        target_link_libraries (HE_debug Ws2_32.lib WSock32.lib)
    endif ()
endif ()
