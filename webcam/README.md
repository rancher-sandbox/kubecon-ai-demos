# Publish a Webcam Stream from a Container Using RTSP 
This component is built with the intent to run it in a kubernetes cluster. The microservice will source a webcam and stream it using the rtsp protocol.

## Dependencies
* Docker (for building the containers)
* [RTSP Simple Server](https://github.com/aler9/rtsp-simple-server)
* Disable secure boot (needed to install the v4l2loopback kernel module)

## Install packages
`zypper in v4l-utils v4l2loopback-kmp-default v4l-tools v4l-utils`

## Developer notes
When pluggin in a webcam, the v4l package is pre loaded as a module in openSUSE Leap 15.4 however, the v4l docs indicated that the preferred package is v4l2. For our purposes, v4l is fine. The main thing is that we need to be consistent and aware of which module is being installed and used. I have installed v4l-tools in order to probe the system while developing since v4l was the default module installed. This info should be useful in capturing the information required to setup Akri or NFD.

1. Plugin the webcam
2. load the v4l2loopback module `modprobe v4l2loopback`

*WARNING:* The webcam must be pluggged in first before the v4l2loopback device is installed. The assumption is that the webcam (streamer) will be on /dev/video0 and the looper will direct output to /dev/video2. If loopback is created before webcam is plugged in, the assumptions may not hold.

## Testing in docker

1. Build the containers
    1. `cd webcam`
    1. `bin/build.sh`
1. Start the streamer in the background (see below)
    * At this point, you can access the webcam over rtsp
1. Start the looper in the background (see below)
    * You should now be able to access the video stream on /dev/video2 as if it were a local usb webcam

### Notes on running the docker containers

* Run the streamer in the backround
    * `docker run --name st1 --rm -d -v /dev/video0:/dev/video0 --network=host --privileged streamer:local`
    * Access the stream at rtsp://localhost:8554/rps

* Run the streamer and login to the container shell
    * `docker run -it --rm --entrypoint=/bin/sh -v /dev/video0:/dev/video0 --network=host --privileged streamer:local`
    * good for debugging tesing streaming changes

* Run the looper in the backround
    * `docker run --name loop1 --rm -d -v /dev/video2:/dev/video2 --network=host --privileged looper:local`
    * Access the stream as if it were a usb camera on /dev/video2

* Run the looper and login to the container shell
    * `docker run -it --rm --entrypoint=/bin/sh -v /dev/video2:/dev/video2 --network=host --privileged looper:local`
    * good for debugging the looper

* Run the restreamer and login to the container shell
    * `docker run -it --rm --entrypoint=/bin/sh -v /dev/video2:/dev/video2 --network=host --privileged restreamer:local`
    * In the container run `./rtsp-simple-server` to start streaming
    * Requires both streamer and looper to be running
    * good for debugging the restreamer
    * streams the video from /dev/video2 
    * Access the stream at rtsp://localhost:9554/rps

### Validate the RTSP stream in ffplay and vlc

* Low latency ffplay 
    * `ffplay -framedrop -flags low_delay -fflags nobuffer -rtsp_transport tcp rtsp://localhost:8554/rps`

* Low latency vlc
    * `vlc --network-caching 0 rtsp://localhost:8554/rps`

### Multicast via udp
**Does not work well over wifi! USE WITH WIRED LAN ONLY**

* vlc
    * `vlc --network-caching 0 rtsp://localhost:8554/rps?vlcmulticast`

* ffplay
    * `ffplay -framedrop -flags low_delay -fflags nobuffer -rtsp_transport udp_multicast rtsp://localhost:8554/rps`