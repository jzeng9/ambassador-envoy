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
        "@envoy//include/envoy/network:connection_interface",
        "@envoy//include/envoy/network:filter_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:logger_lib",
    ],
)

envoy_cc_library(
    name = "extauth_config",
    srcs = ["extauth_config.cc"],
    repository = "@envoy",
    deps = [
        ":extauth_lib",
        "@envoy//include/envoy/network:connection_interface",
        "@envoy//source/server:configuration_lib",
    ],
)

envoy_cc_test(
    name = "extauth_integration_test",
    srcs = ["extauth_integration_test.cc"],
    data =  ["extauth_server.json"],
    repository = "@envoy",
    deps = [
        ":extauth_config",
        "@envoy//test/integration:integration_lib"
    ],
)
