cmake_minimum_required(VERSION 4.2)
project(slipstream)

set(CMAKE_CXX_STANDARD 20)

add_executable(slipstream apps/server.cpp)
add_executable(oe_client apps/oe_client.cpp)
add_executable(md_client apps/md_client.cpp)
add_executable(main main.cpp)

enable_testing()
add_executable(test_codec tests/test_codec.cpp)
add_test(NAME codec COMMAND test_codec)