# Envoy filter example

This project demonstrates the linking of additional filters with the Envoy binary.
A new filter `echo2` is introduced, identical modulo renaming to the existing
[`echo`](https://github.com/envoyproxy/envoy/blob/master/source/extensions/filters/network/echo/echo.h)
filter. Integration tests demonstrating the filter's end-to-end behavior are
also provided.

For an example of additional HTTP filters, see [here](http-filter-example).

## Building

To build the Envoy static binary:

1. `git submodule update --init`
2. `bazel build //:envoy`

## Testing

To run the `echo2` integration test:

`bazel test //:echo2_integration_test`

To run the regular Envoy tests from this project:

`bazel test @envoy//test/...`
