# Created by _as
cmake_policy (SET CMP0025 NEW) # Force CMAKE_<LANG>_COMPILER_ID to differenciate Clang for AppleClang

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # using Clang
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL "8.0.0")
        add_compile_definitions (CTRE_DISABLE_GREEDY_OPT)
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # using GCC
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # using Intel C++
    message (WARNING "Current configured compiler is ICC, which we have not tested. Use at your own risk.")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # using AppleClang
    message (WARNING "Current configured compiler is AppleClang, which we have not tested. Use at your own risk.")
else ()
    message (WARNING "Current configured compiler is ${CMAKE_CXX_COMPILER_ID}, which we do not know of. Use at your own risk.")
endif()
