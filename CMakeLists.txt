
cmake_minimum_required(VERSION 3.20)

project(OptionsPricingEngine
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(options_pricer
    src/main.cpp
    src/black_scholes.cpp
)

target_include_directories(options_pricer
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)  

enable_testing()

add_executable(black_scholes_tests
    tests/test_black_scholes.cpp
    src/black_scholes.cpp
)

target_include_directories(black_scholes_tests
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

add_test(
    NAME BlackScholesTests
    COMMAND black_scholes_tests
)

