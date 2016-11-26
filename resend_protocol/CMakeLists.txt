cmake_minimum_required(VERSION 2.8)

project(resend_protocol)

add_definitions(-DLEIZHOU)

set(CMAKE_CXX_FLAGS -std=c++0x)

set(TOOLKITS_DIR $ENV{TOOLKITS})

include_directories( ../ ../suyun_sdk ../suyun_sdk/adt ../suyun_sdk/adt/linux)

#link_directories(${TOOLKITS_DIR}/lib ../lib)

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(SRC_LIST 
ring_buf_handler.cpp
resend_protocol_send_handler.cpp
resend_protocol_recv_handler.cpp
resend_protocol.cpp
)


add_library(resend_protocol SHARED ${SRC_LIST})

#target_link_libraries(resend_protocol suyun_sdk)
