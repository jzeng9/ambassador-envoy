#!/bin/sh

set -ex

env | grep TRAVIS | sort

# Are we on master?
ONMASTER=

if [ \( "$TRAVIS_BRANCH" = "master" \) -a \( "$TRAVIS_PULL_REQUEST" = "false" \) ]; then
    ONMASTER=yes
fi

# Syntactic sugar really...
onmaster () {
    test -n "$ONMASTER"
}

if onmaster; then
    git checkout ${TRAVIS_BRANCH}
fi

# Perform the build
git submodule update --init
mkdir -p dist
docker run -it --rm -v $TRAVIS_BUILD_DIR:/source \
    lyft/envoy-build:$ENVOY_BUILD_SHA /bin/bash /source/travis-build-docker.sh
docker build -t datawire/ambassador-envoy:latest .
docker tag datawire/ambassador-envoy:latest datawire/ambassador-envoy:$COMMIT

if onmaster; then
    # Avoid `set -x` leaking secret info into Travis logs
    set +x
    echo "+docker login..."
    docker login -u "${DOCKER_USERNAME}" -p "${DOCKER_PASSWORD}"
    set -x
    docker push datawire/ambassador-envoy:latest
    docker push datawire/ambassador-envoy:$COMMIT
else
    echo "not on master; not pushing to Docker Hub"
fi
