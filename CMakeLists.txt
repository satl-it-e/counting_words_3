cmake_minimum_required(VERSION 3.5)
project(counting_words_2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

find_package(Threads REQUIRED)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(ICU_MAJOR_VER "57")
set(ICU_FULL_VERSION "${ICU_MAJOR_VER}.1")


find_package(Boost REQUIRED system)
find_package(Boost COMPONENTS system filesystem locale REQUIRED)
find_package(LibArchive REQUIRED)


add_executable(counting_words_2 test_main.cpp additional.cpp time_measure.h config.cpp my_archive.cpp)


target_link_libraries(counting_words_2  Threads::Threads)
target_link_libraries(counting_words_2 ${LibArchive_LIBRARIES})
target_link_libraries( counting_words_2
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_LOCALE_LIBRARY}
        ${ICU_LIBRARIES})


