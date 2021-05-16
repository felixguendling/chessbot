cmake_minimum_required(VERSION 3.10)
project(chessbot)

include(cmake/pkg.cmake)

find_package(Threads)

file(GLOB_RECURSE chessbot-test-files test/*.cc)

file(GLOB_RECURSE chessbot-files src/*.cc)
add_library(chessbot ${chessbot-files})
target_link_libraries(chessbot boost-filesystem utl cista Threads::Threads)
target_include_directories(chessbot PUBLIC include)
target_compile_features(chessbot PUBLIC cxx_std_20)

file(GLOB perft-files exe/perft.cc)
add_executable(perft ${perft-files})
target_link_libraries(perft chessbot)

file(GLOB filter_pgns-files exe/filter_pgns.cc)
add_executable(filter_pgns ${filter_pgns-files})
target_link_libraries(filter_pgns chessbot)

file(GLOB train_nn-files exe/train_nn.cc)
add_executable(train_nn ${train_nn-files})
target_link_libraries(train_nn chessbot matplot)
target_compile_definitions(train_nn PRIVATE CHESSBOT_PLOT=1)

add_executable(chessbot-test ${chessbot-test-files})
target_link_libraries(chessbot-test boost-filesystem chessbot doctest)

add_executable(chessbot-test-plot ${chessbot-test-files})
target_compile_definitions(chessbot-test-plot PRIVATE CHESSBOT_PLOT=1)
target_link_libraries(chessbot-test-plot boost-filesystem chessbot doctest matplot)


