#[=======================================================================[.rst:
FindYaml
-------

Finds the LibYaml library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Yaml::Yaml``
  The LibYaml library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Yaml_FOUND``
  True if the system has the LibYaml library.
``Yaml_VERSION``
  The version of the LibYaml library which was found.
``Yaml_INCLUDE_DIRS``
  Include directories needed to use LibYaml.
``Yaml_LIBRARIES``
  Libraries needed to link to LibYaml.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Yaml_INCLUDE_DIR``
  The directory containing ``yaml.h``.
``Yaml_LIBRARY``
  The path to the LibYaml library.

#]=======================================================================]
find_package(PkgConfig)
if (PkgConfig_FOUND)
    pkg_check_modules(PC_Yaml QUIET yaml-0.1)
endif()

find_path(Yaml_INCLUDE_DIR NAMES yaml.h PATHS ${PC_Yaml_INCLUDE_DIRS})
find_library(Yaml_LIBRARY NAMES yaml libyaml yaml-static libyaml-static PATHS ${PC_Yaml_LIBRARY_DIRS})
if (PC_Yaml_FOUND)
    set(Yaml_VERSION ${PC_Yaml_VERSION})
    set(Yaml_VERSION_STRING ${Yaml_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Yaml
    FOUND_VAR Yaml_FOUND
    REQUIRED_VARS
        Yaml_LIBRARY
        Yaml_INCLUDE_DIR
    VERSION_VAR Yaml_VERSION
)

if (Yaml_FOUND)
    set(Yaml_LIBRARIES ${Yaml_LIBRARY})
    set(Yaml_INCLUDE_DIRS ${Yaml_INCLUDE_DIR})
    if (PC_Yaml_FOUND)
        set(Yaml_DEFINITIONS ${PC_Yaml_CFLAGS_OTHER})
    endif()
    if (NOT TARGET Yaml::Yaml)
        add_library(Yaml::Yaml UNKNOWN IMPORTED)
        set_target_properties(Yaml::Yaml PROPERTIES
            IMPORTED_LOCATION "${Yaml_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${PC_Yaml_CFLAGS_OTHER}"
            INTERFACE_INCLUDE_DIRECTORIES "${Yaml_INCLUDE_DIR}"
        )
    endif()
endif()
