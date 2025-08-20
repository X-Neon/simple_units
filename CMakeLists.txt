cmake_minimum_required(VERSION 4.0)

project(simple_units VERSION 1.0.0 LANGUAGES CXX)

add_library(simple_units INTERFACE)
target_compile_features(simple_units INTERFACE cxx_std_20)
target_sources(
    simple_units
    INTERFACE FILE_SET
        HEADERS
        BASE_DIRS include
        FILES include/simple_units/unit.hpp
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    simple_unitsConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

add_library(simple_units::simple_units ALIAS simple_units)
install(
    TARGETS simple_units
    EXPORT simple_units_targets
    FILE_SET HEADERS
)
install(
    EXPORT simple_units_targets
    NAMESPACE simple_units::
    FILE simple_unitsConfig.cmake
    DESTINATION lib/cmake/simple_units
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/simple_unitsConfigVersion.cmake
    DESTINATION lib/cmake/simple_units
)

add_executable(example example/example.cpp)
target_link_libraries(example PRIVATE simple_units)

add_executable(simple_units_test test/test.cpp)
target_link_libraries(simple_units_test PRIVATE simple_units)
