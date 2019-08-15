cmake_minimum_required(VERSION 3.12)
project(HeavyEyelid)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

include_directories(thirdparty/gsl/include
        thirdparty/rapidxml_ns
        thirdparty/rapidjson/include
        include)

include(WA-ICEs) # Workarouds for known ICEs on certain compiler versions

if (!MACOS AND !MINGW)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

find_package(Boost 1.66
        REQUIRED
        COMPONENTS system)

find_package(LibVirt
        REQUIRED)

if (Boost_FOUND AND LibVirt_FOUND)
    include_directories(${Boost_INCLUDE_DIRS} ${LibVirt_INCLUDE_DIRS}
            thirdparty/lift/include
            thirdparty/inih
            thirdparty/ctre/include
            include)

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options("-O3")
        add_compile_options("-mtune=native")
        add_compile_options("-g")
    endif ()

    add_executable(HE_debug
            main.cpp
            src/wrapper/depends.cpp
            include/wrapper/http_wrapper.hpp
            include/utils.hpp
            include/virt_wrap/AdminConnection.hpp
            include/virt_wrap/Connection.hpp
            include/virt_wrap/Domain.hpp
            include/virt_wrap/TypesParam.hpp
            include/virt_wrap/decls.hpp
            include/virt_wrap/fwd.hpp
            include/virt_wrap/tfe.hpp
            include/virt_wrap/type_ops.hpp
            include/virt_wrap/utility.hpp
            include/virt_wrap/Error.hpp
            include/virt_wrap/GFlags.hpp
            include/virt_wrap/Network.hpp
            include/virt_wrap/NodeDevice.hpp
            include/virt_wrap/impl/Connection.hpp
            include/virt_wrap/impl/Domain.hpp
            include/virt_wrap/impl/Network.hpp
            include/virt_wrap/impl/TypedParams.hpp
            include/virt_wrap/impl/NodeDevice.hpp
            include/virtxml/capabilities.hpp
            include/virtxml/generic.hpp
            include/virtxml/virtxml.hpp
            include/virtxml/xmlspan.hpp
            include/virtxml/xmlval.hpp
            include/wrapper/actions_table.hpp
            include/wrapper/config.hpp
            include/wrapper/depends.hpp
            include/wrapper/dispatch.hpp
            include/wrapper/error_msg.hpp
            include/wrapper/handler.hpp
            include/wrapper/handlers.hpp
            include/wrapper/solver.hpp
            include/wrapper/handlers/base.hpp
            include/wrapper/handlers/domain.hpp
            include/wrapper/handlers/hdl_ctx.hpp
            include/virt_wrap.hpp
            include/logger.hpp
            include/fwd.hpp
            include/cexpr_algs.hpp
            include/flatmap.hpp
            include/json_utils.hpp
            include/urlparser.hpp include/wrapper/handlers/network.hpp)

    target_link_libraries(HE_debug ${Boost_LIBRARIES} ${LibVirt_LIBRARIES} pthread)
    if (WIN32)
        target_link_libraries(HE_debug Ws2_32.lib WSock32.lib)
    endif ()
endif ()
