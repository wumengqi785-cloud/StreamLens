add_executable(streamlens_smoke_test
    smoke_test.cpp
)

target_include_directories(streamlens_smoke_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(streamlens_smoke_test PRIVATE cxx_std_17)

add_test(NAME streamlens_smoke_test COMMAND streamlens_smoke_test)
