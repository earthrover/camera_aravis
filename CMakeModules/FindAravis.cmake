
find_package(PkgConfig QUIET)

if( PKG_CONFIG_FOUND )
  pkg_check_modules( Aravis aravis-0.8 )
endif()

if( NOT Aravis_FOUND )
  message("Aravis (aravis-0.8) could not be found by pkg-config. Trying to manually find Aravis.")

  find_path(Aravis_INCLUDE_DIRS arv.h
    PATHS
    "$ENV{ARAVIS_INCLUDE_PATH}"
    /usr/local/include/aravis-0.8
    /usr/include/aravis-0.8
    /usr/local/include/aravis-0.7
    /usr/include/aravis-0.7
    /usr/local/include/aravis-0.6
    /usr/include/aravis-0.6
  )


  find_library(Aravis_LIBRARIES NAMES aravis-0.8 aravis-0.7 aravis-0.6
    PATHS
    "$ENV{ARAVIS_LIBRARY}"
    /usr/local/lib
    /usr/lib
    /usr/lib/x86_64-linux-gnu
  )

  list(APPEND Aravis_LIBRARIES "glib-2.0;gmodule-2.0;gobject-2.0")

  get_filename_component(Aravis_LIBS_DIR "${Aravis_LIBRARIES}" DIRECTORY)

  list(APPEND Aravis_INCLUDE_DIRS "${Aravis_LIBS_DIR};${GLIB2_INCLUDE_DIRS}")


  set(Aravis_VERSION_REGEX "#define ARAVIS_(MAJOR|MINOR|MICRO)_VERSION[ \t]+([0-9]+)")

  file(STRINGS "${Aravis_LIBS_DIR}/arvversion.h" Aravis_VERSION_FILE REGEX ${Aravis_VERSION_REGEX})

  string(REGEX REPLACE "${Aravis_VERSION_REGEX}" "\\2" Aravis_VERSION "${Aravis_VERSION_FILE}")
  string(REGEX REPLACE ";" "." Aravis_VERSION "${Aravis_VERSION}")

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args( Aravis
    REQUIRED_VARS Aravis_INCLUDE_DIRS Aravis_LIBRARIES
    VERSION_VAR Aravis_VERSION
  )
endif()

message( STATUS "Aravis found: ${Aravis_FOUND} version: ${Aravis_VERSION}")

if ( Aravis_FOUND AND NOT Aravis_VERSION )
  message( WARNING "Aravis version not detected")
endif()