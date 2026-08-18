cmake_minimum_required(VERSION 4.2)
project(orderbook)

set(CMAKE_CXX_STANDARD 23)

add_executable(
        main main.cpp
        include/order.h
        include/orderbook.h
)

### Catch2 Testing Framework
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)

FetchContent_MakeAvailable(Catch2)

enable_testing()
add_subdirectory(tests)

