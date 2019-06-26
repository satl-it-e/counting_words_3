cmake_minimum_required(VERSION 3.5)
project(counting_words_2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

find_package(Threads REQUIRED)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(ICU_MAJOR_VER "57")
set(ICU_FULL_VERSION "${ICU_MAJOR_VER}.1")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Boost REQUIRED system)
find_package(Boost COMPONENTS system filesystem locale REQUIRED)
find_package(LibArchive REQUIRED)
find_package(TBB REQUIRED)


include_directories( ${TBB_INCLUDE_DIR} )
include_directories(thread_pool)

add_executable(counting_words_3 main.cpp additional.cpp config.cpp my_archive.cpp additional.h config.h my_archive.h thread_pool/ctpl.h)


target_link_libraries(counting_words_3  Threads::Threads)
target_link_libraries(counting_words_3 ${LibArchive_LIBRARIES})
target_link_libraries( counting_words_3
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_LOCALE_LIBRARY}
        ${ICU_LIBRARIES})

target_link_libraries(counting_words_3 tbb)


