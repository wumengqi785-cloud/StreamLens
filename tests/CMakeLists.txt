add_executable(streamlens_smoke_test
    smoke_test.cpp
)

target_include_directories(streamlens_smoke_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(streamlens_smoke_test PRIVATE cxx_std_17)

add_test(NAME streamlens_smoke_test COMMAND streamlens_smoke_test)

add_executable(streamlens_config_test
    config_test.cpp
    ${PROJECT_SOURCE_DIR}/src/config.cpp
)

target_include_directories(streamlens_config_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(streamlens_config_test PRIVATE cxx_std_17)

add_test(NAME streamlens_config_test COMMAND streamlens_config_test)

add_executable(streamlens_rtp_parser_test
    rtp_parser_test.cpp
    ${PROJECT_SOURCE_DIR}/src/rtp/rtp_parser.cpp
)

target_include_directories(streamlens_rtp_parser_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(streamlens_rtp_parser_test PRIVATE cxx_std_17)

add_test(NAME streamlens_rtp_parser_test COMMAND streamlens_rtp_parser_test)

add_executable(streamlens_statistics_test
    stream_statistics_test.cpp
    ${PROJECT_SOURCE_DIR}/src/statistics/stream_statistics.cpp
)

target_include_directories(streamlens_statistics_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(streamlens_statistics_test PRIVATE cxx_std_17)

add_test(NAME streamlens_statistics_test COMMAND streamlens_statistics_test)
