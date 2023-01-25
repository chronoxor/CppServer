if(NOT TARGET Catch2)

  # Restore origin compile flags
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_ORIGIN}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_ORIGIN}")

  # Module subdirectory
  add_subdirectory("Catch2")

  # Module folder
  set_target_properties(Catch2 PROPERTIES FOLDER "modules/Catch2")
  set_target_properties(Catch2WithMain PROPERTIES FOLDER "modules/Catch2")

  # Restore custom compile flags
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_CUSTOM}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_CUSTOM}")

endif()
