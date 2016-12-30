if(NOT TARGET asio)

  # External packages
  if(MSVC)
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openssl")
  endif()
  set(OPENSSL_USE_STATIC_LIBS TRUE)
  find_package(OpenSSL REQUIRED)

  # Module library
  file(GLOB SOURCE_FILES "asio/asio/src/*.cpp")
  add_library(asio ${SOURCE_FILES})
  target_compile_definitions(asio PRIVATE ASIO_STANDALONE ASIO_SEPARATE_COMPILATION)
  target_include_directories(asio PRIVATE "asio/asio/include" ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(asio OpenSSL::SSL)

  # Module folder
  set_target_properties(asio PROPERTIES FOLDER modules/asio)

endif()
