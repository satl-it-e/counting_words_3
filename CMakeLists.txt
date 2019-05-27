cmake_minimum_required(VERSION 3.5)
project(counting_words_3)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Threads REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Concurrent)

add_executable(counting_words_3 main.cpp additional.cpp config.cpp)

target_link_libraries(counting_words_3 Qt5::Concurrent)
target_link_libraries(counting_words_3  Threads::Threads)



#find_package(LibArchive REQUIRED)
#target_link_libraries(counting_words_3 ${LibArchive_LIBRARIES})


