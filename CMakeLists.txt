cmake_minimum_required(VERSION 3.10)
project(chessbot)

include(cmake/pkg.cmake)

find_package(Threads)

file(GLOB_RECURSE chessbot-files src/*.cc)
file(GLOB_RECURSE chessbot-test-files test/*.cc)
file(GLOB main-file src/main.cc)
list(REMOVE_ITEM chessbot-files ${main-file})

add_library(chessbot ${chessbot-files})
target_link_libraries(chessbot utl cista)
target_include_directories(chessbot PUBLIC include)
target_compile_features(chessbot PUBLIC cxx_std_20)

add_executable(chessbot-exe ${chessbot-files})
target_link_libraries(chessbot-exe chessbot)

add_executable(chessbot-test ${chessbot-test-files})
target_link_libraries(chessbot-test chessbot doctest Threads::Threads)


