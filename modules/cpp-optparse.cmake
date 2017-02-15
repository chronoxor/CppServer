if(NOT TARGET cpp-optparse)

  # Module library
  file(GLOB SOURCE_FILES "cpp-optparse/OptionParser.cpp")
  set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
  add_library(cpp-optparse ${SOURCE_FILES})

  # Module folder
  set_target_properties(cpp-optparse PROPERTIES FOLDER modules/cpp-optparse)

endif()
