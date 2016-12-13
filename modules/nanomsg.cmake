if(NOT TARGET nanomsg)

  # Module flags
  set(NN_HAVE_CONDVAR 1)

  # Module options
  SET(NN_STATIC_LIB ON CACHE BOOL "Build static library instead of shared library")
  SET(NN_ENABLE_DOC OFF CACHE BOOL "Disable building documentation")
  SET(NN_TESTS OFF CACHE BOOL "Disable building nanomsg tests")

  # Module subdirectory
  add_subdirectory("nanomsg")

  # Module folder
  set_target_properties(nanomsg PROPERTIES FOLDER modules/nanomsg)
  set_target_properties(nanocat PROPERTIES FOLDER modules/nanomsg)
  set_target_properties(dist PROPERTIES FOLDER modules/nanomsg)

endif()
