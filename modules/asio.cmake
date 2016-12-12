if(NOT TARGET asio)

  # Save old variables
  set(CMAKE_CXX_FLAGS_OLD ${CMAKE_CXX_FLAGS})

  # Module library
  file(GLOB SOURCE_FILES "asio/asio/src/*.cpp")
  if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "")
  else()
    # C4127: conditional expression is constant
    # C4702: unreachable code
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127 /wd4702")
  endif()
  add_library(asio ${SOURCE_FILES})
  target_compile_definitions(asio PRIVATE ASIO_STANDALONE=1 ASIO_SEPARATE_COMPILATION=1)
  target_include_directories(asio PRIVATE "asio/asio/include" ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(asio)

  # Module folder
  set_target_properties(asio PROPERTIES FOLDER modules/asio)

  # Restore old variables
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_OLD})

endif()
