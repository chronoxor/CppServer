if(NOT TARGET cpp-optparse)

  # Module library
  file(GLOB SOURCE_FILES "cpp-optparse/OptionParser.cpp")
  add_library(cpp-optparse ${SOURCE_FILES})
  if(MSVC)
    # C4244: 'conversion' conversion from 'type1' to 'type2', possible loss of data
    # C4996: <header> is removed in C++20
    set_target_properties(cpp-optparse PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS} /wd4244 /wd4996")
  else()
    set_target_properties(cpp-optparse PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
  endif()
  target_include_directories(cpp-optparse PUBLIC "cpp-optparse")

  # Module folder
  set_target_properties(cpp-optparse PROPERTIES FOLDER "modules/cpp-optparse")

endif()
