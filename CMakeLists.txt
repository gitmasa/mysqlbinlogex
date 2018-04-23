cmake_minimum_required(VERSION 2.8)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#find_package(Regex REQUIRED)

project(mysqlbinlogex)
set(VERSION "0.3.1")

configure_file(src/version.hpp.in src/version.hpp)

set(SOURCE_FILES
        src/filemanage.cpp
        src/logparser.cpp
        src/my_datetime.cpp
        src/mysqlbinlogex.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Wno-long-long -pedantic -lboost_regex")

add_executable(mysqlbinlogex ${SOURCE_FILES})

#target_link_libraries(mysqlbinlogex std::regex)
