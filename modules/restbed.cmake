if(NOT TARGET restbed)

  # Module library
  file(GLOB_RECURSE SOURCE_FILES "restbed/source/corvusoft/restbed/*.cpp")
  add_library(restbed ${SOURCE_FILES})
  target_compile_definitions(restbed PRIVATE ASIO_STANDALONE ASIO_SEPARATE_COMPILATION BUILD_SSL)
  target_include_directories(restbed PRIVATE "restbed/source" PRIVATE "restbed/dependency/kashmir" PRIVATE "asio/asio/include" PRIVATE ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(restbed ${OPENSSL_LIBRARIES})

  # MinGW build should be manually linked with Winsock2 library
  if(WIN32)
    if(NOT MSVC)
      target_link_libraries(restbed wsock32 ws2_32)
    endif()
  endif()

  # Module folder
  set_target_properties(restbed PROPERTIES FOLDER modules/restbed)

endif()
