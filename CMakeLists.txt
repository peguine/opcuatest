cmake_minimum_required(VERSION 3.15)
project(opcuatest)

set(CMAKE_CXX_STANDARD 14)


if(WIN32)
    link_directories(D:/doc/c++/libs/opcua)
    include_directories(D:/doc/c++/libs/opcua)
else()
    link_directories(D:/doc/c++/libs/arm_libs/opcua)
    include_directories(D:/doc/c++/libs/arm_libs/opcua)
endif()

add_executable(opcuatest OpcUaTest.cpp)
target_link_libraries(opcuatest open62541)
add_executable(subscripetest subscripetest.cpp)
target_link_libraries(subscripetest open62541)
add_executable(writetest writetest.cpp)
target_link_libraries(writetest open62541)