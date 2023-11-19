if(NOT TARGET Catch2)

  # Module library
  file(GLOB SOURCE_FILES "Catch2/extras/catch_amalgamated.cpp")
  add_library(Catch2 ${SOURCE_FILES})
  target_include_directories(Catch2 PUBLIC "Catch2/extras")

  # Module folder
  set_target_properties(Catch2 PROPERTIES FOLDER "modules/Catch2")

endif()
