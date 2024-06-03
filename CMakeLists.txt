cmake_minimum_required(VERSION 3.5)

project(module28)

set(SRC_LIST 
          main.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})

set(CXX_FLAGS "${CXX_FLAGS} -std=c++17 -Wall")