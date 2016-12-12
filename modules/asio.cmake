if(NOT TARGET asio)

  # Module library
  file(GLOB SOURCE_FILES "asio/asio/src/*.cpp")
  if(NOT MSVC)
    set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "")
  else()
    # C4127: conditional expression is constant
    # C4702: unreachable code
    set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS} /wd4127 /wd4702")
  endif()
  add_library(asio ${SOURCE_FILES})
  target_compile_definitions(asio PRIVATE ASIO_STANDALONE=1 ASIO_SEPARATE_COMPILATION=1)
  target_include_directories(asio PRIVATE "asio/asio/include" "openssl/include")
  target_link_libraries(asio)

  # Module folder
  set_target_properties(asio PROPERTIES FOLDER modules/asio)

endif()
