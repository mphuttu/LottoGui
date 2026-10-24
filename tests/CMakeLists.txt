cmake_minimum_required(VERSION 3.20)

project(LottoGuiRegressionTests LANGUAGES CXX)

include(FetchContent)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723
)
FetchContent_MakeAvailable(googletest)

add_executable(LottoGuiRegressionTests
    LottoGuiCoreTests.cpp
    ../LottoGui/LottoGuiCore.cpp
)
target_include_directories(LottoGuiRegressionTests PRIVATE ../LottoGui)
target_compile_features(LottoGuiRegressionTests PRIVATE cxx_std_14)
target_link_libraries(LottoGuiRegressionTests PRIVATE GTest::gtest_main)

include(CTest)
if(BUILD_TESTING)
    add_test(NAME LottoGuiRegressionTests COMMAND LottoGuiRegressionTests)
endif()
