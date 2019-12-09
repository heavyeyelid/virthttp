#  Defines the follwing variables:
#  LibDeflate_FOUND - system has LibDeflate
#  LibDeflate_INCLUDE_DIRS - the LibDeflate include directories
#  LibDeflate_LIBRARIES - link these to use LibDeflate

include(LibFindMacros)

# Include dir
find_path(LibDeflate_INCLUDE_DIR
        NAMES libdeflate.h
        )

# Finally the library itself
find_library(LibDeflate_LIBRARY
        NAMES libdeflate.a
        )

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LibDeflate_PROCESS_INCLUDES LibDeflate_INCLUDE_DIR)
set(LibDeflate_PROCESS_LIBS LibDeflate_LIBRARY)
libfind_process(LibDeflate)
