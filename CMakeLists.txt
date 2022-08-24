CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
PROJECT (ftpserver)

SET (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET (SOURCES
        ${SRC_DIR}/servercore.cpp
        ${SRC_DIR}/serverconnection.cpp
        ${SRC_DIR}/fileoperator.cpp
        ${SRC_DIR}/ftpserver.cpp
    )

ADD_EXECUTABLE (${PROJECT_NAME} ${SOURCES})
