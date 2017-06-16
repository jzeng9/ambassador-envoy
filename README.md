# Ambassador Envoy

This repo contains Ambassador's external authentication filter for [Lyft Envoy][en]. The Travis configuration for this repo builds an Envoy binary (at tag 1.3.0) that includes the ExtAuth filter. It then creates a [Docker image for Ambassador][ad] using that binary.

[en]: https://lyft.github.io/envoy/
[ad]: https://hub.docker.com/r/datawire/ambassador-envoy/tags/

See [Ambassador][ag] and [its documentation][aw] for more information.

[ag]: https://github.com/datawire/ambassador
[aw]: http://www.getambassador.io/


## ExtAuth

When ExtAuth receives a client request, it asks the configured auth service what to do with the client request. ExtAuth sends a POST request to the auth service at the path `/ambassador/auth` with body content containing a JSON mapping of the request headers in HTTP2 style, e.g., `:authority` instead of `Host`. If ExtAuth cannot reach the auth service, it returns 503 to the client. If the auth service response code is 200, then ExtAuth allows the client request to be resume being processed by the normal Envoy flow. This will typically mean that the client will receive the expected response to its request. If ExtAuth receives any response from the auth service other than 200, it returns that full response (header and body) to the client. ExtAuth assumes the auth service will return an appropriate response, such as 401.


## Building

If you don't have a [build environment for Envoy][be], you can use Docker to set one up.

[be]: https://github.com/lyft/envoy/blob/master/bazel/README.md

    $ fgrep ENVOY_BUILD_SHA .travis.yml
      - ENVOY_BUILD_SHA=fc747b3c2fd49b1260484572071fe4194cd6824d

    $ docker run -it -p 9999:9999 --name envoy-build lyft/envoy-build:fc747b3c2fd49b1260484572071fe4194cd6824d /bin/bash

    root@e30a286b80c5:/# cd

Once your build environment is ready, grab a copy of the source code and build.

    root@e30a286b80c5:~# git clone https://github.com/datawire/ambassador-envoy
    Cloning into 'ambassador-envoy'...
    remote: Counting objects: 144, done.
    remote: Compressing objects: 100% (18/18), done.
    remote: Total 144 (delta 7), reused 20 (delta 4), pack-reused 121
    Receiving objects: 100% (144/144), 40.43 KiB | 0 bytes/s, done.
    Resolving deltas: 100% (66/66), done.
    Checking connectivity... done.

    root@e30a286b80c5:~# cd ambassador-envoy

    root@e30a286b80c5:~/ambassador-envoy# git submodule update --init
    Submodule 'envoy' (https://github.com/lyft/envoy.git) registered for path 'envoy'
    Cloning into 'envoy'...
    remote: Counting objects: 72332, done.
    remote: Compressing objects: 100% (191/191), done.
    remote: Total 72332 (delta 127), reused 28 (delta 7), pack-reused 72132
    Receiving objects: 100% (72332/72332), 24.44 MiB | 6.75 MiB/s, done.
    Resolving deltas: 100% (60008/60008), done.
    Checking connectivity... done.
    Submodule path 'envoy': checked out '3afc7712a04907ffd25ed497626639febfe65735'

    root@e30a286b80c5:~/ambassador-envoy# bazel build -c dbg //:envoy
    [... many lines of Bazel output ...]
    INFO: Found 1 target...
    Target //:envoy up-to-date:
      bazel-bin/envoy
    INFO: Elapsed time: 657.911s, Critical Path: 238.94s


## Testing

The Envoy binary is found in the `bazel-bin` directory. Launch Envoy using the sample configuration as follows. The redirect/pipe through `egrep` serves to highlight log output from the ExtAuth filter. This can make it easier to spot relevant messages.

    root@e30a286b80c5:~/ambassador-envoy# bazel-bin/envoy -c extauth_server.json -l info 2>&1 | egrep --color 'ExtAuth|$'
    [2017-06-14 20:51:49.758][30594][warning][main] initializing epoch 0 (hot restart version=8.2490552)
    [2017-06-14 20:51:49.768][30594][info][main] admin address: 127.0.0.1:0
    [2017-06-14 20:51:49.770][30594][info][upstream] cm init: adding: cluster=httpbin primary=1 secondary=0
    [2017-06-14 20:51:49.772][30594][info][upstream] cm init: adding: cluster=auth primary=2 secondary=0
    [2017-06-14 20:51:49.772][30594][info][config] loading 1 listener(s)
    [2017-06-14 20:51:49.772][30594][info][config] listener #0:
    [2017-06-14 20:51:49.772][30594][info][config]   address=0.0.0.0:9999
    [2017-06-14 20:51:49.772][30594][info][config]   filter #0:
    [2017-06-14 20:51:49.772][30594][info][config]     type: read
    [2017-06-14 20:51:49.772][30594][info][config]     name: http_connection_manager
    [2017-06-14 20:51:49.775][30594][info][config]     filter #0
    [2017-06-14 20:51:49.775][30594][info][config]       type: decoder
    [2017-06-14 20:51:49.775][30594][info][config]       name: extauth
    [2017-06-14 20:51:49.775][30594][info][config]     filter #1
    [2017-06-14 20:51:49.775][30594][info][config]       type: decoder
    [2017-06-14 20:51:49.775][30594][info][config]       name: router
    [2017-06-14 20:51:49.775][30594][info][config] loading tracing configuration
    [2017-06-14 20:51:49.775][30594][warning][main] starting main dispatch loop
    [2017-06-14 20:51:49.794][30594][info][upstream] cm init: removing: cluster=auth primary=1 secondary=0
    [2017-06-14 20:51:49.794][30594][info][upstream] cm init: removing: cluster=httpbin primary=0 secondary=0
    [2017-06-14 20:51:49.794][30594][warning][main] all clusters initialized. initializing init manager
    [2017-06-14 20:51:49.794][30594][warning][main] all dependencies initialized. starting workers
    [2017-06-14 20:51:49.795][30597][info][main] worker entering dispatch loop
    [2017-06-14 20:51:49.795][30598][info][main] worker entering dispatch loop

Now you can access Envoy and the services configured under it on port 9999 from another terminal:

    $ curl -v 127.0.0.1:9999/service/get
    *   Trying 127.0.0.1...
    * TCP_NODELAY set
    * Connected to 127.0.0.1 (127.0.0.1) port 9999 (#0)
    > GET /get HTTP/1.1
    > Host: 127.0.0.1:9999
    > User-Agent: curl/7.51.0
    > Accept: */*
    >
    < HTTP/1.1 503 Service Unavailable
    < content-length: 57
    < content-type: text/plain
    < date: Wed, 14 Jun 2017 20:54:16 GMT
    < server: envoy
    <
    * Curl_http_done: called premature == 0
    * Connection #0 to host 127.0.0.1 left intact
    upstream connect error or disconnect/reset before headers

Envoy returns 503 Service Unavailable because it cannot reach the configured auth server. A look at Envoy's output makes this evident:

    [2017-06-14 20:53:41.225][30598][info][main] [C1] new connection
    [2017-06-14 20:53:41.227][30598][info][filter] ExtAuth Request received; contacting auth server
    [2017-06-14 20:53:41.228][30598][info][client] [C2] connecting
    [2017-06-14 20:53:41.228][30598][info][filter] ExtAuth Auth responded with code 503
    [2017-06-14 20:53:41.228][30598][info][filter] ExtAuth Auth said: upstream connect error or disconnect/reset before headers
    [2017-06-14 20:53:41.228][30598][info][filter] ExtAuth rejecting request
    [2017-06-14 20:53:41.230][30598][info][main] [C1] adding to cleanup list

Let's launch the [sample auth server][as].

[as]: https://github.com/datawire/ambassador-auth-service

    $ docker pull datawire/ambassador-auth-service:latest
    latest: Pulling from datawire/ambassador-auth-service
    2aecc7e1714b: Pull complete
    8c9904a62f4c: Pull complete
    03e43cd0c4c3: Pull complete
    ea2ca032df77: Pull complete
    cf5c747aca5b: Pull complete
    Digest: sha256:78c46829e124be43a6976fea53a6e120f6c9ce24ef68782bdedf55d7acd4b9c5
    Status: Downloaded newer image for datawire/ambassador-auth-service:latest

    $ docker run -it --rm -p 3000:3000 datawire/ambassador-auth-service:latest

    > authserver@1.0.0 start /src
    > node server.js

    Example app listening on port 3000!

Now try to access your service without credentials:

    $ curl -v 127.0.0.1:9999/service/get
    *   Trying 127.0.0.1...
    * TCP_NODELAY set
    * Connected to 127.0.0.1 (127.0.0.1) port 9999 (#0)
    > GET /service/get HTTP/1.1
    > Host: 127.0.0.1:9999
    > User-Agent: curl/7.51.0
    > Accept: */*
    >
    < HTTP/1.1 401 Unauthorized
    < x-powered-by: Express
    < www-authenticate: Basic realm="Ambassador Realm"
    < date: Wed, 14 Jun 2017 21:26:17 GMT
    < content-length: 0
    < x-envoy-upstream-service-time: 28
    < server: envoy
    <
    * Curl_http_done: called premature == 0
    * Connection #0 to host 127.0.0.1 left intact

You receive the exact output of the auth service, in this case a 401. Try again with credentials:

    $ curl -u username:password -v 127.0.0.1:9999/service/get
    *   Trying 127.0.0.1...
    * TCP_NODELAY set
    * Connected to 127.0.0.1 (127.0.0.1) port 9999 (#0)
    * Server auth using Basic with user 'username'
    > GET /service/get HTTP/1.1
    > Host: 127.0.0.1:9999
    > Authorization: Basic dXNlcm5hbWU6cGFzc3dvcmQ=
    > User-Agent: curl/7.51.0
    > Accept: */*
    >
    < HTTP/1.1 200 OK
    < server: envoy
    < date: Wed, 14 Jun 2017 21:26:23 GMT
    < content-type: application/json
    < access-control-allow-origin: *
    < access-control-allow-credentials: true
    < x-powered-by: Flask
    < x-processed-time: 0.00169396400452
    < content-length: 364
    < via: 1.1 vegur
    < x-envoy-upstream-service-time: 472
    <
    {
      "args": {},
      "headers": {
        "Accept": "*/*",
        "Authorization": "Basic dXNlcm5hbWU6cGFzc3dvcmQ=",
        "Connection": "close",
        "Host": "httpbin.org",
        "User-Agent": "curl/7.51.0",
        "X-Envoy-Expected-Rq-Timeout-Ms": "15000",
        "X-Envoy-Original-Path": "/service/get"
      },
      "origin": "204.148.29.50",
      "url": "http://httpbin.org/get"
    }
    * Curl_http_done: called premature == 0
    * Connection #0 to host 127.0.0.1 left intact

You receive the output of your service, which in this example is [httpbin.org](http://httpbin.org/).

The Envoy side of things looks as you would expect. No credentials:

    [2017-06-14 21:26:17.859][30598][info][main] [C20] new connection
    [2017-06-14 21:26:17.860][30598][info][filter] ExtAuth Request received; contacting auth server
    [2017-06-14 21:26:17.860][30598][info][client] [C21] connecting
    [2017-06-14 21:26:17.889][30598][info][filter] ExtAuth Auth responded with code 401
    [2017-06-14 21:26:17.889][30598][info][filter] ExtAuth rejecting request
    [2017-06-14 21:26:17.891][30598][info][main] [C20] adding to cleanup list

With credentials:

    [2017-06-14 21:26:23.603][30597][info][main] [C22] new connection
    [2017-06-14 21:26:23.604][30597][info][filter] ExtAuth Request received; contacting auth server
    [2017-06-14 21:26:23.604][30597][info][client] [C23] connecting
    [2017-06-14 21:26:23.615][30597][info][filter] ExtAuth Auth responded with code 200
    [2017-06-14 21:26:23.615][30597][info][filter] ExtAuth Auth said: OK
    [2017-06-14 21:26:23.615][30597][info][filter] ExtAuth accepting request
    [2017-06-14 21:26:23.616][30597][info][client] [C24] connecting
    [2017-06-14 21:26:24.091][30597][info][main] [C22] adding to cleanup list


## How the build works

The [Envoy repository](https://github.com/lyft/envoy/) is provided as a submodule.
The [`WORKSPACE`](WORKSPACE) file maps the `@envoy` repository to this local path.

The [`BUILD`](BUILD) file introduces a new Envoy static binary target, `envoy`,
that links together the new filter and `@envoy//source/exe:envoy_main_lib`. The
ExtAuth filter registers itself during the static initialization phase of the
Envoy binary as a new filter.
