if(NOT TARGET cppbenchmark)

  # Module flag
  set(CPPBENCHMARK_MODULE Y)

  # Module subdirectory
  add_subdirectory("CppBenchmark")

  # Module folder
  set_target_properties(cppbenchmark PROPERTIES FOLDER "modules/CppBenchmark")

endif()
