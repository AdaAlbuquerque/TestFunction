CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(OUTPUT_OBJECT boostTest)

PROJECT(${OUTPUT_OBJECT})

SET(${OUTPUT_OBJECT}_VERSION 0.1)

INCLUDE_DIRECTORIES(
    ./include
)

LINK_DIRECTORIES(
    /usr/local/lib
    /usr/lib
    ${PROJECT_SOURCE_DIR}/lib
)

SET(CMAKE_BUILD_TYPE DEBUG)

SET(LIBS )

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/main)
