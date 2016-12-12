if(NOT TARGET nanomsg)

  # Save old compiler flags
  set(CMAKE_C_FLAGS_OLD ${CMAKE_C_FLAGS})

  # Setup new compiler flags
  if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-aliasing=n")
  else()
    # Disable some compile warnings
    # C4100: 'identifier' : unreferenced formal parameter
    # C4152: non standard extension, function/data ptr conversion in expression
    # C4201: nonstandard extension used : nameless struct/union
    # C4206: nonstandard extension used : translation unit is empty
    # C4244: 'conversion' conversion from 'type1' to 'type2', possible loss of data
    # C4245: 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
    # C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
    # C4295: 'array' : array is too small to include a terminating null character
    # C4459: declaration of 'identifier' hides global declaration
    # C4701: Potentially uninitialized local variable 'name' used
    # C4702: unreachable code
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4100 /wd4152 /wd4201 /wd4206 /wd4244 /wd4245 /wd4267 /wd4295 /wd4459 /wd4701 /wd4702")
  endif()

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

  # Restore old compiler flags
  set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_OLD})

endif()
