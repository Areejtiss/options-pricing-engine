
cmake_minimum_required(VERSION 3.20)

project(OptionsPricingEngine
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(quant STATIC
    src/black_scholes.cpp
    src/greeks.cpp
)

target_include_directories(quant
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

add_executable(options_pricer
    src/main.cpp
)

target_link_libraries(options_pricer PRIVATE quant)

enable_testing()

add_executable(black_scholes_tests
    tests/test_black_scholes.cpp
)

target_link_libraries(black_scholes_tests PRIVATE quant)

add_test(
    NAME BlackScholesTests
    COMMAND black_scholes_tests
)

add_executable(greeks_tests
    tests/test_greeks.cpp
)

target_link_libraries(greeks_tests PRIVATE quant)

add_test(
    NAME GreeksTests
    COMMAND greeks_tests
)
