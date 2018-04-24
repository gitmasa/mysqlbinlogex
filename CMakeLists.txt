cmake_minimum_required(VERSION 2.8)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(Boost REQUIRED)

project(mysqlbinlogex)
set(VERSION "0.3.1")

configure_file(src/version.hpp.in src/version.hpp)

set(SOURCE_FILES
        src/filemanage.cpp
        src/logparser.cpp
        src/my_datetime.cpp
        src/mysqlbinlogex.cpp
)

set(SOURCE_FILES2
        src/filemanage.cpp
        src/txtlogparser.cpp
        src/my_datetime.cpp
        src/mysqltxtlog.cpp
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Wno-long-long -pedantic")

add_executable(mysqlbinlogex ${SOURCE_FILES})
add_executable(mysqltxtlog ${SOURCE_FILES2})

target_link_libraries(mysqlbinlogex boost_regex)
target_link_libraries(mysqltxtlog boost_regex)


set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
        "${CMAKE_PROJECT_NAME}-${VERSION}")

set(CPACK_SOURCE_IGNORE_FILES
        "/build/;/.bzr/;/.idea/;/CMakeFiles/;/.git/;/CMakeCache.txt;/.gitignore;/CPackConfig.cmake;/CPackSourceConfig.cmake;/Makefile;/cmake_install.cmake;/text_commands.txt;~$;${CPACK_SOURCE_IGNORE_FILES}")

include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

INSTALL(TARGETS mysqlbinlogex RUNTIME DESTINATION bin)
INSTALL(TARGETS mysqltxtlog RUNTIME DESTINATION bin)
