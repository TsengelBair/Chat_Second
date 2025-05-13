cmake_minimum_required(VERSION 3.14)

project(Server LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GENERATED_PB_FILES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ProtoFiles")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network Sql REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network Sql REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIRS})
file(GLOB_RECURSE PROTO_FILES ${GENERATED_PB_FILES_DIR}/*.pb.cc ${GENERATED_PB_FILES_DIR}/*.pb.h)

set(PROJECT_SOURCES
    main.cpp
    server.h
    server.cpp
    validator.h
    validator.cpp
    serializer.h
    serializer.cpp
    dbhandler.h
    dbhandler.cpp
    packetbuilder.h
    packetbuilder.cpp
    types.h
    ${PROTO_FILES}
)

add_executable(Server ${PROJECT_SOURCES})

target_link_libraries(Server PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Sql ${PROTOBUF_LIBRARIES})
