# Publish a Webcam Stream from a Container Using RTSP 
This component is built with the intent to run it in a kubernetes cluster. The microservice will source a webcam and stream it using the rtsp protocol.

## Dependencies
Docker daemon
[RTSP Simple Server](https://github.com/aler9/rtsp-simple-server)

## Developer notes
When pluggin in a webcam, the v4l package is pre loaded as a module in openSUSE Leap 15.4 however, the v4l docs indicated that the preferred package is v4l2. For our purposes, v4l is fine. The main thing is that we need to be consistent and aware of which module is being installed and used. I have installed v4l-tools in order to probe the system while developing since v4l was the default module installed. This info should be useful in capturing the information required to setup Akri or NFD.

### Testing in docker
`docker run -it --rm --entrypoint=/bin/sh -v /dev/video0:/dev/video0 --network=host --privileged rtsp-streamer:local`
`docker run --name st1 --rm -d -v /dev/video0:/dev/video0 --network=host --privileged streamer:local`
`docker run -it --rm --entrypoint=/bin/sh -v /dev/video2:/dev/video2 --network=host --privileged looper:local`

### Validate the RTSP stream in ffplay and vlc
`ffplay -framedrop -flags low_delay -fflags nobuffer -rtsp_transport tcp rtsp://localhost:8554/rps`
or
`vlc --network-caching 0 rtsp://localhost:8554/rps`