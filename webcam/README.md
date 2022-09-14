# Publish a Webcam Stream from a Container Using RTSP 
This component is built with the intent to run it in a kubernetes cluster. The microservice will source a webcam and stream it using the rtsp protocol.

## Dependencies
Docker daemon
[RTSP Simple Server](https://github.com/aler9/rtsp-simple-server)

## Developer notes
When pluggin in a webcam, the v4l package is pre loaded as a module in openSUSE Leap 15.4 however, the v4l docs indicated that the preferred package is v4l2. For our purposes, v4l is fine. The main thing is that we need to be consistent and aware of which module is being installed and used. I have installed v4l-tools in order to probe the system while developing since v4l was the default module installed. This info should be useful in capturing the information required to setup Akri or NFD.

### Testing in docker

* Run the streamer and login to the container shell
    * `docker run -it --rm --entrypoint=/bin/sh -v /dev/video0:/dev/video0 --network=host --privileged streamer:local`
    * good for debugging tesing streaming changes

* Run the streamer in the backround
    * `docker run --name st1 --rm -d -v /dev/video0:/dev/video0 --network=host --privileged streamer:local`
    * Access the stream at rtsp://localhost:8554/rps

* Run the looper and login to the container shell
    * `docker run -it --rm --entrypoint=/bin/sh -v /dev/video2:/dev/video2 --network=host --privileged looper:local`
    * good for debugging the looper

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