if (LIBUV_DIR)
  SET(LIBUV_PATHMODE "NO_DEFAULT_PATH")
else ()
  SET(LIBUV_PATHMODE "")
endif ()

find_path(LIBUV_INCLUDE_DIR uv.h PATHS ${LIBUV_DIR} ${LIBUV_DIR}/include ${LIBUV_PATHMODE})
find_library(LIBUV_LIBRARIES NAMES uv PATHS ${LIBUV_DIR} ${LIBUV_DIR}/lib ${LIBUV_PATHMODE})

if (LIBUV_INCLUDE_DIR AND LIBUV_LIBRARIES)
  if (NOT TARGET libuv::uv)
    add_library(libuv::uv UNKNOWN IMPORTED)
    set_target_properties(libuv::uv PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LIBUV_INCLUDE_DIR} IMPORTED_LOCATION ${LIBUV_LIBRARIES})
  endif ()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libuv REQUIRED_VARS LIBUV_LIBRARIES LIBUV_INCLUDE_DIR)

if(LIBUV_FOUND)
  message(STATUS "Found libuv - (include: ${LIBUV_INCLUDE_DIR}, library: ${LIBUV_LIBRARIES})")
  mark_as_advanced(LIBUV_INCLUDE_DIR LIBUV_LIBRARIES)
endif()
