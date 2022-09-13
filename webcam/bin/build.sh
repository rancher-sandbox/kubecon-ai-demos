#!/bin/sh -x

TAG=$([ -z "$1" ] && echo "local" || echo "$1")
DOCKER_USER=$([ -z "$2" ] && echo "" || echo "$2/")

docker build -t "$DOCKER_USER"rtsp-streamer:"$TAG" .