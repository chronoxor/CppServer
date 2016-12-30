cmake_minimum_required(VERSION 2.8)

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
include(SetPlatformFeatures)
if(MSVC)
  # C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")
endif()

# Modules
add_subdirectory("modules")

# External packages
if(MSVC)
  set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/openssl")
endif()
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
if(UNIX)
  find_package(LibBFD)
  find_package(LibDL)
  find_package(LibRT)
  find_package(LibUUID)
endif()
if(WIN32)
  find_package(Crypt)
  find_package(DbgHelp)
  find_package(Userenv)
endif()

# Link libraries
list(APPEND LINKLIBS ${OPENSSL_LIBRARIES})
list(APPEND LINKLIBS Threads::Threads)
if(UNIX)
  list(APPEND LINKLIBS ${LIBBFD_LIBRARIES})
  list(APPEND LINKLIBS ${LIBDL_LIBRARIES})
  list(APPEND LINKLIBS ${LIBRT_LIBRARIES})
  list(APPEND LINKLIBS ${LIBUUID_LIBRARIES})
endif()
if(WIN32)
  list(APPEND LINKLIBS ${CRYPT_LIBRARIES})
  list(APPEND LINKLIBS ${DBGHELP_LIBRARIES})
  list(APPEND LINKLIBS ${USERENV_LIBRARIES})
endif()
list(APPEND LINKLIBS cppcommon)

# Library
file(GLOB_RECURSE SOURCE_FILES "source/*.cpp")
set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
add_library(cppserver ${SOURCE_FILES})
target_include_directories(cppserver PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/modules/asio/asio/include" PUBLIC ${OPENSSL_INCLUDE_DIR})
target_link_libraries(cppserver ${LINKLIBS} asio nanomsg)
set_target_properties(cppserver PROPERTIES FOLDER libraries)
list(APPEND INSTALL_TARGETS cppserver)
list(APPEND LINKLIBS cppserver)

# Additional module components: examples, performance benchmarks, tests, tools and install
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

  # Performance benchmarks
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
  add_executable(cppserver-tests ${CATCH} ${TESTS_SOURCE_FILES})
  target_include_directories(cppserver-tests PRIVATE "modules/catch/single_include")
  target_link_libraries(cppserver-tests ${LINKLIBS})
  set_target_properties(cppserver-tests PROPERTIES FOLDER tests)
  list(APPEND INSTALL_TARGETS cppserver-tests)
  list(APPEND INSTALL_TARGETS_PDB cppserver-tests)

  # CTest
  enable_testing()
  add_test(cppserver-tests cppserver-tests)

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
