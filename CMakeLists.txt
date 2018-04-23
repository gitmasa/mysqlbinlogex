cmake_minimum_required(VERSION 2.8)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#find_package(Regex REQUIRED)

project(mysqlbinlogex)

set(SOURCE_FILES
        src/filemanage.cpp
        src/logparser.cpp
        src/my_datetime.cpp
        src/mysqlbinlogex.cpp
        src/mysqltxtlog.cpp
        src/txtlogparser.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long -pedantic")

add_executable(mysqlbinlogex ${SOURCE_FILES})

#target_link_libraries(mysqlbinlogex std::regex)
