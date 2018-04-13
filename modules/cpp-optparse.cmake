if(NOT TARGET cpp-optparse)

  # Module library
  file(GLOB SOURCE_FILES "cpp-optparse/OptionParser.cpp")
  if(NOT MSVC)
    set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
  else()
    # C4244: 'conversion' conversion from 'type1' to 'type2', possible loss of data
    set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS} /wd4244")
  endif()
  add_library(cpp-optparse ${SOURCE_FILES})
  target_include_directories(cpp-optparse PUBLIC "cpp-optparse")

  # Module folder
  set_target_properties(cpp-optparse PROPERTIES FOLDER modules/cpp-optparse)

endif()
