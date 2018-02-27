if(NOT TARGET asio)

  # Module library
  file(GLOB SOURCE_FILES "asio/asio/src/*.cpp")
  if(NOT MSVC)
    set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
  else()
    # C4996: The compiler encountered a deprecated declaration
    set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS} /wd4996")
  endif()
  add_library(asio ${SOURCE_FILES})
  target_compile_definitions(asio PRIVATE ASIO_STANDALONE ASIO_SEPARATE_COMPILATION)
  target_include_directories(asio PRIVATE "asio/asio/include" PRIVATE ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(asio ${OPENSSL_LIBRARIES})

  # Module folder
  set_target_properties(asio PROPERTIES FOLDER modules/asio)

endif()
