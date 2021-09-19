#[=======================================================================[.rst:
FindPCRE
-------

Finds the LibPCRE library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``PCRE::PCRE``
  The LibPCRE library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PCRE_FOUND``
  True if the system has the LibPCRE library.
``PCRE_VERSION``
  The version of the LibPCRE library which was found.
``PCRE_INCLUDE_DIRS``
  Include directories needed to use LibPCRE.
``PCRE_LIBRARIES``
  Libraries needed to link to LibPCRE.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``PCRE_INCLUDE_DIR``
  The directory containing ``pcre.h``.
``PCRE_LIBRARY``
  The path to the LibPCRE library.

#]=======================================================================]
find_package(PkgConfig)
if (PkgConfig_FOUND)
    pkg_check_modules(PC_PCRE QUIET libpcre)
endif()

find_path(PCRE_INCLUDE_DIR NAMES "pcre.h" PATHS ${PC_PCRE_INCLUDE_DIRS})
find_library(PCRE_LIBRARY NAMES pcre libpcre PATHS ${PC_PCRE_LIBRARY_DIRS})
if (PC_PCRE_FOUND)
    set(PCRE_VERSION ${PC_PCRE_VERSION})
    set(PCRE_VERSION_STRING ${PCRE_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE
    FOUND_VAR PCRE_FOUND
    REQUIRED_VARS
        PCRE_LIBRARY
        PCRE_INCLUDE_DIR
    VERSION_VAR PCRE_VERSION
)

if (PCRE_FOUND)
    set(PCRE_LIBRARIES ${PCRE_LIBRARY})
    set(PCRE_INCLUDE_DIRS ${PCRE_INCLUDE_DIR})
    if (PC_PCRE_FOUND)
        set(PCRE_DEFINITIONS ${PC_PCRE_CFLAGS_OTHER})
    endif()
    if (NOT TARGET PCRE::PCRE)
        add_library(PCRE::PCRE UNKNOWN IMPORTED)
        set_target_properties(PCRE::PCRE PROPERTIES
            IMPORTED_LOCATION "${PCRE_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_PCRE_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${PCRE_INCLUDE_DIR}"
        )
    endif()
endif()
