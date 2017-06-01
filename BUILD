package(default_visibility = ["//visibility:public"])

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
    "envoy_cc_test",
)

envoy_cc_binary(
    name = "envoy",
    repository = "@envoy",
    deps = [
        ":extauth_config",
        "@envoy//source/exe:envoy_main_lib",
    ],
)

envoy_cc_library(
    name = "extauth_lib",
    srcs = ["extauth.cc"],
    hdrs = ["extauth.h"],
    repository = "@envoy",
    deps = [
        "@envoy//include/envoy/buffer:buffer_interface",
        "@envoy//include/envoy/http:filter_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:logger_lib",
        "@envoy//source/common/http:headers_lib",
        "@envoy//source/common/http:header_map_lib"
    ],
)

envoy_cc_library(
    name = "extauth_config",
    srcs = ["extauth_config.cc"],
    hdrs = ["extauth_config.h"],
    repository = "@envoy",
    deps = [
        ":extauth_lib",
        "@envoy//source/server:configuration_lib",
        "@envoy//source/server/config/network:http_connection_manager_lib"
    ],
)

envoy_cc_test(
    name = "extauth_integration_test",
    srcs = ["extauth_integration_test.cc"],
    data =  ["extauth_server.json"],
    repository = "@envoy",
    deps = [
        ":extauth_config",
        "@envoy//test/mocks/buffer:buffer_mocks",
        "@envoy//test/mocks/http:http_mocks",
        "@envoy//test/mocks/upstream:upstream_mocks"
    ],
)
