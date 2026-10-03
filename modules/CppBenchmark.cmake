if(CPPSERVER_BENCHMARKS AND NOT TARGET cppbenchmark AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CppBenchmark/CMakeLists.txt")

  # Module flag
  set(CPPBENCHMARK_MODULE Y)

  # Module subdirectory
  add_subdirectory("CppBenchmark")

  # Module folder
  set_target_properties(cppbenchmark PROPERTIES FOLDER "modules/CppBenchmark")

endif()

