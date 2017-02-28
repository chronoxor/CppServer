if(NOT TARGET restbed)

  # Module library
  file(GLOB_RECURSE SOURCE_FILES "restbed/source/corvusoft/restbed/*.cpp")
  add_library(restbed ${SOURCE_FILES})
  target_compile_definitions(restbed PRIVATE ASIO_STANDALONE ASIO_SEPARATE_COMPILATION)
  target_include_directories(restbed PRIVATE "restbed/source" PRIVATE "restbed/dependency/kashmir" PRIVATE "asio/asio/include" PRIVATE ${OPENSSL_INCLUDE_DIR})
  #target_link_libraries(asio ${OPENSSL_LIBRARIES})

  # Module folder
  set_target_properties(restbed PROPERTIES FOLDER modules/restbed)

endif()
