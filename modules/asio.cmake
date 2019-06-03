if(NOT TARGET asio AND NOT CYGWIN)

  # Module library
  file(GLOB SOURCE_FILES "asio/asio/src/*.cpp")
  add_library(asio ${SOURCE_FILES})
  target_compile_definitions(asio PRIVATE ASIO_STANDALONE ASIO_SEPARATE_COMPILATION)
  target_include_directories(asio PUBLIC "asio/asio/include" PUBLIC ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(asio ${OPENSSL_LIBRARIES})

  # Module folder
  set_target_properties(asio PROPERTIES FOLDER modules/asio)

endif()
