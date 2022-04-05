cmake_minimum_required( VERSION 3.10 )

project( frn VERSION 0.1 DESCRIPTION "Fully Replicated MPC for N parties" )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O2 -march=native -Wall -Wextra -pedantic -Werror -std=gnu++17")

set(EXP_INPUT "exp_input.x")
set(EXP_MULT "exp_mult.x")
set(EXP_CHECK "exp_check.x")
set(TEST_EXECUTABLE "tests.x")

set(SOURCE_FILES
  src/frn/lib/primitives/prg.cc
  src/frn/lib/tools.cc
  src/frn/lib/math/arithmetic.cc
  src/frn/lib/net/builder.cc
  src/frn/lib/net/channel.cc
  src/frn/lib/net/connector.cc
  src/frn/lib/net/sysi.cc
  src/frn/shr.cc
  src/frn/input.cc
  src/frn/input_corr.cc
  src/frn/corr.cc
  src/frn/mult.cc
  src/frn/check.cc)

set(TEST_SOURCE_FILES
  test/main.cc
  test/mock_network.cc
  test/test_corr.cc
  test/test_mult.cc
  test/test_tcp_network.cc
  test/test_check.cc
  test/test_shr.cc
  test/test_input.cc)	

include_directories(src)

find_package( Catch2 REQUIRED )
include( CTest )
include( Catch )

add_executable( ${TEST_EXECUTABLE} ${SOURCE_FILES} ${TEST_SOURCE_FILES} )
target_link_libraries( ${TEST_EXECUTABLE} Catch2::Catch2 pthread)
catch_discover_tests( ${TEST_EXECUTABLE} )

add_executable(${EXP_INPUT} src/frn/exp_input.cc ${SOURCE_FILES})
target_link_libraries(${EXP_INPUT} pthread)
add_executable(${EXP_MULT} src/frn/exp_mult.cc ${SOURCE_FILES})
target_link_libraries(${EXP_MULT} pthread)
add_executable(${EXP_CHECK} src/frn/exp_check.cc ${SOURCE_FILES})
target_link_libraries(${EXP_CHECK} pthread)
