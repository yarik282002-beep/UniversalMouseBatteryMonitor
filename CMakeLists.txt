cmake_minimum_required(VERSION 3.20)
project(UniversalMouseBatteryMonitor VERSION 0.3.0 LANGUAGES CXX)

option(UMBM_BUILD_TESTS "Build portable unit tests" ON)

add_library(umbm_battery_math INTERFACE)
target_include_directories(umbm_battery_math INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(umbm_battery_math INTERFACE cxx_std_20)

if(WIN32)
    add_executable(UniversalMouseBatteryMonitor WIN32
        src/main.cpp
    )
    target_link_libraries(UniversalMouseBatteryMonitor PRIVATE
        setupapi hid hidparse shell32 user32 gdi32 advapi32
    )
    target_compile_definitions(UniversalMouseBatteryMonitor PRIVATE
        UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
        _WIN32_WINNT=0x0A00 WINVER=0x0A00
    )
    if(MSVC)
        target_compile_options(UniversalMouseBatteryMonitor PRIVATE /W4 /permissive- /EHsc /utf-8)
        target_link_options(UniversalMouseBatteryMonitor PRIVATE /SUBSYSTEM:WINDOWS)
    endif()
else()
    message(STATUS "The tray/HID application is Windows-only; portable tests can still be built.")
endif()

if(UMBM_BUILD_TESTS)
    enable_testing()
    add_executable(umbm_battery_math_tests tests/BatteryMathTests.cpp)
    target_link_libraries(umbm_battery_math_tests PRIVATE umbm_battery_math)
    add_test(NAME BatteryMathTests COMMAND umbm_battery_math_tests)
endif()
