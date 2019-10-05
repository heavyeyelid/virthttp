cmake_minimum_required(VERSION 3.12)
project(HeavyEyelid)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

add_subdirectory(thirdparty/virtxml++)

include_directories(thirdparty/gsl/include
        thirdparty/rapidjson/include
        thirdparty
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

    add_compile_definitions(VIR_ENUM_SENTINELS RAPIDJSON_HAS_STDSTRING)

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options("-O3")
        add_compile_options("-mtune=native")
        add_compile_options("-g")
    endif ()

    add_executable(virthttp
            main.cpp
            src/wrapper/depends.cpp
            src/wrapper/json2virt.cpp
            include/utils.hpp
            include/virt_wrap/AdminConnection.hpp
            include/virt_wrap/Connection.hpp
            include/virt_wrap/Domain.hpp
            include/virt_wrap/TypesParam.hpp
            include/virt_wrap/decls.hpp
            include/virt_wrap/fwd.hpp
            include/virt_wrap/tfe.hpp
            include/virt_wrap/utility.hpp
            include/virt_wrap/Error.hpp
            include/virt_wrap/GFlags.hpp
            include/virt_wrap/Network.hpp
            include/virt_wrap/NodeDevice.hpp
            include/virt_wrap/CpuMap.hpp
            include/virt_wrap/Stream.hpp
            include/virt_wrap/impl/Stream.hpp
            include/virt_wrap/enums/Domain/Domain.hpp
            include/virt_wrap/enums/Base.hpp
            include/virt_wrap/enums/Domain/BlockCommitFlag.hpp
            include/virt_wrap/enums/Domain/BlockCopyFlag.hpp
            include/virt_wrap/enums/Domain/BlockJobAbortFlag.hpp
            include/virt_wrap/enums/Domain/BlockJobInfoFlag.hpp
            include/virt_wrap/enums/Domain/BlockJobSetSpeedFlag.hpp
            include/virt_wrap/enums/Domain/BlockPullFlag.hpp
            include/virt_wrap/enums/Domain/BlockRebaseFlag.hpp
            include/virt_wrap/enums/Domain/BlockResizeFlag.hpp
            include/virt_wrap/enums/Domain/ChannelFlag.hpp
            include/virt_wrap/enums/Domain/ConsoleFlag.hpp
            include/virt_wrap/enums/Domain/CreateFlag.hpp
            include/virt_wrap/enums/Domain/DestroyFlag.hpp
            include/virt_wrap/enums/Domain/DeviceModifyFlag.hpp
            include/virt_wrap/enums/Domain/GetJobStatsFlag.hpp
            include/virt_wrap/enums/Domain/KeycodeSet.hpp
            include/virt_wrap/enums/Domain/Lifecycle.hpp
            include/virt_wrap/enums/Domain/LifecycleAction.hpp
            include/virt_wrap/enums/Domain/MemoryModFlag.hpp
            include/virt_wrap/enums/Domain/OpenGraphicsFlag.hpp
            include/virt_wrap/enums/Domain/ShutdownFlag.hpp
            include/virt_wrap/enums/Domain/MetadataType.hpp
            include/virt_wrap/enums/Domain/MigrateFlag.hpp
            include/virt_wrap/enums/Domain/ModificationImpactFlag.hpp
            include/virt_wrap/enums/Domain/ProcessSignal.hpp
            include/virt_wrap/enums/Domain/XmlFlag.hpp
            include/virt_wrap/enums/Domain/State.hpp
            include/virt_wrap/enums/Domain/UndefineFlag.hpp
            include/virt_wrap/enums/Domain/SaveRestoreFlag.hpp
            include/virt_wrap/enums/Connection/ListDomainFlag.hpp
            include/virt_wrap/enums/Connection/Connection.hpp
            include/virt_wrap/enums/Stream/Stream.hpp
            include/virt_wrap/enums/Stream/Flag.hpp
            include/virt_wrap/enums/Stream/RecvFlag.hpp
            include/virt_wrap/impl/Connection.hpp
            include/virt_wrap/impl/Domain.hpp
            include/virt_wrap/impl/Network.hpp
            include/virt_wrap/impl/TypedParams.hpp
            include/virt_wrap/impl/NodeDevice.hpp
            include/wrapper/actions_table.hpp
            include/wrapper/config.hpp
            include/wrapper/depends.hpp
            include/wrapper/dispatch.hpp
            include/wrapper/error_msg.hpp
            include/wrapper/handler.hpp
            include/wrapper/json2virt.hpp
            include/wrapper/http_wrapper.hpp
            include/wrapper/solver.hpp
            include/wrapper/virt2json.hpp
            include/wrapper/handlers/base.hpp
            include/wrapper/handlers/domain.hpp
            include/wrapper/handlers/flagwork.hpp
            include/wrapper/handlers/hdl_ctx.hpp
            include/wrapper/handlers/network.hpp
            include/wrapper/general_store.hpp
            include/wrapper/handlers/async/async_handler.hpp
            include/wrapper/handlers/async/async_store.hpp
            include/wrapper/protocol_support/beast_internals.hpp
            include/wrapper/protocol_support/protocols.hpp
            include/wrapper/protocol_support/request_handler.hpp
            include/wrapper/protocol_support/tcp_listener.hpp
            include/wrapper/protocol_support/http1/Session.hpp
            include/virt_wrap.hpp
            include/logger.hpp
            include/cexpr_algs.hpp
            include/flatmap.hpp
            include/json_utils.hpp
            include/urlparser.hpp

            include/wrapper/decoder_support/zlib_beast.hpp include/wrapper/decoder_support/zlib_beast_impl.hpp include/wrapper/decoder_support/zlib_beast_detail.hpp)

    target_link_libraries(virthttp virtxml++ ${Boost_LIBRARIES} ${LibVirt_LIBRARIES} pthread)
    if (WIN32)
        target_link_libraries(virthttp Ws2_32.lib WSock32.lib)
    endif ()
endif ()