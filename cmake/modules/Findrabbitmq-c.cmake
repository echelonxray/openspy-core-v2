if (RABBITMQ_DIR)
  SET(RABBITMQ_PATHMODE "NO_DEFAULT_PATH")
else ()
  SET(RABBITMQ_PATHMODE "")
endif ()

find_path(RABBITMQ_INCLUDE_DIR amqp.h rabbitmq-c/amqp.h PATHS ${RABBITMQ_DIR} ${RABBITMQ_DIR}/include ${RABBITMQ_PATHMODE})
find_library(RABBITMQ_LIBRARIES NAMES rabbitmq PATHS ${RABBITMQ_DIR} ${RABBITMQ_DIR}/lib ${RABBITMQ_DIR}/lib/x86_64-linux-gnu ${RABBITMQ_PATHMODE})

if (RABBITMQ_INCLUDE_DIR AND RABBITMQ_LIBRARIES)
  if (NOT TARGET rabbitmq::rabbitmq)
    add_library(rabbitmq::rabbitmq UNKNOWN IMPORTED)
    set_target_properties(rabbitmq::rabbitmq PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${RABBITMQ_INCLUDE_DIR} IMPORTED_LOCATION ${RABBITMQ_LIBRARIES})
  endif ()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(rabbitmq NAME_MISMATCHED REQUIRED_VARS RABBITMQ_LIBRARIES RABBITMQ_INCLUDE_DIR)

if(RABBITMQ_FOUND)
  message(STATUS "Found rabbitmq - (include: ${RABBITMQ_INCLUDE_DIR}, library: ${RABBITMQ_LIBRARIES})")
  mark_as_advanced(RABBITMQ_INCLUDE_DIR RABBITMQ_LIBRARIES)
endif()
