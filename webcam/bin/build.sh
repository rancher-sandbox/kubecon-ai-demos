#!/bin/sh -x

TAG=$([ -z "$1" ] && echo "local" || echo "$1")
DOCKER_USER=$([ -z "$2" ] && echo "" || echo "$2/")

docker build -t "$DOCKER_USER"streamer:"$TAG" -f dockerfiles/Dockerfile.streamer .
docker build -t "$DOCKER_USER"looper:"$TAG" -f dockerfiles/Dockerfile.looper .