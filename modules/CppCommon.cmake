if(NOT TARGET cppcommon)

  # Module flag
  set(CPPCOMMON_MODULE Y)

  # Module subdirectory
  add_subdirectory("CppCommon")

  # Module folder
  set_target_properties(cppcommon PROPERTIES FOLDER "modules/CppCommon")

endif()
