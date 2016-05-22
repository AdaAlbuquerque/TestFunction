
CMAKE_MINIMUM_REQUIRED(VERSION 2.8 )

SET(OUTPUT_OBJECT functionTest )

PROJECT(${OUTPUT_OBJECT} )

SET(${OUTPUT_OBJECT}_VERSION 0.1)

SET(CODE_DIR ${PROJECT_SOURCE_DIR})

INCLUDE_DIRECTORIES(
    /usr/local/include
    /usr/local/include/zookeeper
    ${CODE_DIR}/include
    )

LINK_DIRECTORIES(
    /usr/local/lib
    /usr/lib
    ${CODE_DIR}/lib
)

SET(CMAKE_BUILD_TYPE Debug)

SET(LIBS )

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#ADD_SUBDIRECTORY(${CODE_DIR}/main)
ADD_SUBDIRECTORY(${CODE_DIR}/src/server)
ADD_SUBDIRECTORY(${CODE_DIR}/src/client)

