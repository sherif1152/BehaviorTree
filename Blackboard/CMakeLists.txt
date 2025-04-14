cmake_minimum_required(VERSION 3.10)
project(Blackboard)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(simple_seq_bt)

find_package(behaviortree_cpp REQUIRED)

add_executable(main_bt main.cpp )

target_link_libraries(main_bt behaviortree_cpp::behaviortree_cpp )
