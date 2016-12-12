if(NOT TARGET asio)

  # Module library
  file(GLOB SOURCE_FILES "asio/asio/src/*.cpp")
  add_library(asio ${SOURCE_FILES})
  target_compile_definitions(asio PRIVATE ASIO_STANDALONE=1 ASIO_SEPARATE_COMPILATION=1)
  target_include_directories(asio PRIVATE "asio/asio/include" ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(asio)

  # Module folder
  set_target_properties(asio PROPERTIES FOLDER modules/asio)

endif()
