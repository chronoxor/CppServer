cmake_minimum_required(VERSION 3.6)

# Global properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Project name
project(cppserver)

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  set(DOXYGEN "doxygen")
  if(NOT TARGET ${DOXYGEN})
    add_custom_command(OUTPUT "Doxyfile" COMMAND ${DOXYGEN_EXECUTABLE} "Doxyfile" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/documents")
    add_custom_target(${DOXYGEN} DEPENDS "Doxyfile")
    set_target_properties(${DOXYGEN} PROPERTIES FOLDER doxygen)
  endif()
endif()

# CMake module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Compiler features
include(SetCompilerFeatures)
include(SetCompilerWarnings)
include(SetPlatformFeatures)
include(SystemInformation)

# External packages
set(OPENSSL_USE_STATIC_LIBS TRUE)
if(MSVC)
  set(OPENSSL_MSVC_STATIC_RT TRUE)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)")
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/openssl/Win32")
  else()
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/openssl/Win64")
  endif()
endif()
find_package(OpenSSL REQUIRED)
if(WIN32)
  find_package(Crypt)
  find_package(WinSock)
endif()

# Modules
add_subdirectory("modules")

# Link libraries
list(APPEND LINKLIBS ${OPENSSL_LIBRARIES})
if(WIN32)
  list(APPEND LINKLIBS ${CRYPT_LIBRARIES})
  list(APPEND LINKLIBS ${WINSOCK_LIBRARIES})
endif()
list(APPEND LINKLIBS cppcommon)

# System directories
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/modules")

# Library
file(GLOB_RECURSE SOURCE_FILES "source/*.cpp")
set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
add_library(cppserver ${SOURCE_FILES})
target_include_directories(cppserver PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(cppserver ${LINKLIBS} asio)
set_target_properties(cppserver PROPERTIES FOLDER libraries)
list(APPEND INSTALL_TARGETS cppserver)
list(APPEND LINKLIBS cppserver)

# Additional module components: benchmarks, examples, plugins, tests, tools and install
if(NOT CPPSERVER_MODULE)

  # Examples
  file(GLOB EXAMPLE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/examples" "examples/*.cpp")
  foreach(EXAMPLE_FILE ${EXAMPLE_FILES})
    string(REGEX REPLACE "(.*)\\.cpp" "\\1" EXAMPLE_NAME ${EXAMPLE_FILE})
    set(EXAMPLE_TARGET "cppserver-example-${EXAMPLE_NAME}")
    set_source_files_properties(examples/${EXAMPLE_FILE} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
    add_executable(${EXAMPLE_TARGET} examples/${EXAMPLE_FILE})
    target_link_libraries(${EXAMPLE_TARGET} ${LINKLIBS})
    set_target_properties(${EXAMPLE_TARGET} PROPERTIES FOLDER examples)
    list(APPEND INSTALL_TARGETS ${EXAMPLE_TARGET})
    list(APPEND INSTALL_TARGETS_PDB ${EXAMPLE_TARGET})
  endforeach()

  # Benchmarks
  file(GLOB BENCHMARK_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/performance" "performance/*.cpp")
  foreach(BENCHMARK_FILE ${BENCHMARK_FILES})
    string(REGEX REPLACE "(.*)\\.cpp" "\\1" BENCHMARK_NAME ${BENCHMARK_FILE})
    set(BENCHMARK_TARGET "cppserver-performance-${BENCHMARK_NAME}")
    set_source_files_properties(performance/${BENCHMARK_FILE} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
    add_executable(${BENCHMARK_TARGET} performance/${BENCHMARK_FILE})
    target_link_libraries(${BENCHMARK_TARGET} ${LINKLIBS} cppbenchmark)
    set_target_properties(${BENCHMARK_TARGET} PROPERTIES FOLDER performance)
    list(APPEND INSTALL_TARGETS ${BENCHMARK_TARGET})
    list(APPEND INSTALL_TARGETS_PDB ${BENCHMARK_TARGET})
  endforeach()

  # Tests
  file(GLOB TESTS_SOURCE_FILES "tests/*.cpp")
  set_source_files_properties(${TESTS_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
  add_executable(cppserver-tests ${Catch2} ${TESTS_SOURCE_FILES})
  target_include_directories(cpplogging-tests PRIVATE ${Catch2})
  target_link_libraries(cppserver-tests ${LINKLIBS})
  set_target_properties(cppserver-tests PROPERTIES FOLDER tests)
  list(APPEND INSTALL_TARGETS cppserver-tests)
  list(APPEND INSTALL_TARGETS_PDB cppserver-tests)

  # CTest
  enable_testing()
  add_test(cppserver-tests cppserver-tests --durations yes --order lex)

  # Install
  install(TARGETS ${INSTALL_TARGETS}
    RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/bin"
    LIBRARY DESTINATION "${PROJECT_SOURCE_DIR}/bin"
    ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/bin")

  # Install *.pdb files
  if(MSVC)
    foreach(INSTALL_TARGET_PDB ${INSTALL_TARGETS_PDB})
      install(FILES $<TARGET_PDB_FILE:${INSTALL_TARGET_PDB}> DESTINATION "${PROJECT_SOURCE_DIR}/bin")
    endforeach()
  endif()

endif()
