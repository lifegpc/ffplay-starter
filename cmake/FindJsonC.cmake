#[=======================================================================[.rst:
FindJsonC
-------

Finds the Libjson-c library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``JsonC::JsonC``
  The Libjson-c library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``JsonC_FOUND``
  True if the system has the Libjson-c library.
``JsonC_VERSION``
  The version of the Libjson-c library which was found.
``JsonC_INCLUDE_DIRS``
  Include directories needed to use Libjson-c.
``JsonC_LIBRARIES``
  Libraries needed to link to Libjson-C.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``JsonC_INCLUDE_DIR``
  The directory containing ``json-c/json.h``.
``JsonC_LIBRARY``
  The path to the Libjson-c library.

#]=======================================================================]
find_package(PkgConfig)
if (PkgConfig_FOUND)
    pkg_check_modules(PC_JsonC QUIET json-c)
endif()

find_path(JsonC_INCLUDE_DIR NAMES "json-c/json.h" PATHS ${PC_JsonC_INCLUDE_DIRS})
find_library(JsonC_LIBRARY NAMES json-c libjson-c json-c-static libjson-c-static PATHS ${PC_JsonC_LIBRARY_DIRS})
if (PC_JsonC_FOUND)
    set(JsonC_VERSION ${PC_JsonC_VERSION})
    set(JsonC_VERSION_STRING ${JsonC_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JsonC
    FOUND_VAR JsonC_FOUND
    REQUIRED_VARS
        JsonC_LIBRARY
        JsonC_INCLUDE_DIR
    VERSION_VAR JsonC_VERSION
)

if (JsonC_FOUND)
    set(JsonC_LIBRARIES ${JsonC_LIBRARY})
    set(JsonC_INCLUDE_DIRS ${JsonC_INCLUDE_DIR})
    if (PC_JsonC_FOUND)
        set(JsonC_DEFINITIONS ${PC_JsonC_CFLAGS_OTHER})
    endif()
    if (NOT TARGET JsonC::JsonC)
        add_library(JsonC::JsonC UNKNOWN IMPORTED)
        set_target_properties(JsonC::JsonC PROPERTIES
            IMPORTED_LOCATION "${JsonC_LIBRARY}"
#            INTERFACE_COMPILE_OPTIONS "${PC_JsonC_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${JsonC_INCLUDE_DIR}"
        )
    endif()
endif()
