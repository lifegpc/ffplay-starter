#[=======================================================================[.rst:
FindChardet
-------

Finds the Libchardet library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Chardet::Chardet``
  The Libchardet library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Chardet_FOUND``
  True if the system has the Libchardet library.
``Chardet_VERSION``
  The version of the Libchardet library which was found.
``Chardet_INCLUDE_DIRS``
  Include directories needed to use Libchardet.
``Chardet_LIBRARIES``
  Libraries needed to link to Libchardet.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Chardet_INCLUDE_DIR``
  The directory containing ``chardet.h``.
``Chardet_LIBRARY``
  The path to the Libchardet library.

#]=======================================================================]
find_package(PkgConfig)
if (PkgConfig_FOUND)
    pkg_check_modules(PC_Chardet QUIET chardet)
endif()

find_path(Chardet_INCLUDE_DIR NAMES "chardet/chardet.h" PATHS ${PC_Chardet_INCLUDE_DIRS})
find_library(Chardet_LIBRARY NAMES chardet libchardet chardet-static libchardet-static PATHS ${PC_Chardet_LIBRARY_DIRS})
if (PC_Chardet_FOUND)
    set(Chardet_VERSION ${PC_Chardet_VERSION})
    set(Chardet_VERSION_STRING ${Chardet_VERSION})
endif()
if (EXISTS "${Chardet_INCLUDE_DIR}/chardet/version.h")
    file(STRINGS "${Chardet_INCLUDE_DIR}/chardet/version.h" TEMPChardet_VERSION
    REGEX "^#[\t ]*define[\t ]+LIBCHARDET_VERSION[\t ]+\"[0-9.]+\"$")
    if (NOT "${TEMPChardet_VERSION}" STREQUAL "")
        string(REGEX MATCH "[0-9.]+" TEMPChardet_VERSION ${TEMPChardet_VERSION})
        set(Chardet_VERSION ${TEMPChardet_VERSION})
        set(Chardet_VERSION_STRING ${Chardet_VERSION})
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Chardet
    FOUND_VAR Chardet_FOUND
    REQUIRED_VARS
        Chardet_LIBRARY
        Chardet_INCLUDE_DIR
    VERSION_VAR Chardet_VERSION
)

if (Chardet_FOUND)
    set(Chardet_LIBRARIES ${Chardet_LIBRARY})
    set(Chardet_INCLUDE_DIRS ${Chardet_INCLUDE_DIR})
    if (PC_Chardet_FOUND)
        set(Chardet_DEFINITIONS ${PC_Chardet_CFLAGS_OTHER})
    endif()
    if (NOT TARGET Chardet::Chardet)
        add_library(Chardet::Chardet UNKNOWN IMPORTED)
        set_target_properties(Chardet::Chardet PROPERTIES
            IMPORTED_LOCATION "${Chardet_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_Chardet_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${Chardet_INCLUDE_DIR}"
        )
    endif()
endif()
