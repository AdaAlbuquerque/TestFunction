CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(OUTPUT_OBJECT zookeeperTest)

INCLUDE_DIRECTORIES(
    .
)

AUX_SOURCE_DIRECTORY(. SRC_LIST)

MESSAGE("SRC LIST: \n" ${SRC_LIST})

SET(LIBS    #event 
            boost_system 
            boost_thread 
            #json_linux-gcc-4.8.3_libmt 
            hpr 
           # rdkafka 
            zookeeper_mt
            #hlog
            #curl
            uuid
            )

IF(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS} ${DEBUGFLAGS}  -std=c++11")
ELSE(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS}  -std=c++11")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    MESSAGE("BUILD TYPE: "${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS})

ADD_EXECUTABLE(${OUTPUT_OBJECT} ${SRC_LIST})

TARGET_LINK_LIBRARIES(${OUTPUT_OBJECT} ${LIBS})
