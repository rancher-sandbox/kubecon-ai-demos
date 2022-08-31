# kubecon-ai-demos

## Rock Paper Scissor Game

### Goals

* Build an ML inference library to interpret human hand gestures
* Build a computer UI for gameplay

    * 3..2..1..go
    * Visual display of computer move
    * Reset game
    * Humans play multiple rounds (i.e. best of 10)
* Build a humanoid robotic arm to show computer moves
* All gameplay components run in containers

    * camera feed (rtsp feed preferred to onboard integration)
    * Inference algorithm
    * Robot arm gestures
    * Gameplay UI
    * Message bus
* Deliver the solution as an open source project in a public git repo
* Demo the cluster(s) connected via Rancher
* Demo gitops updates/rollbacks for various components

### System

* Jetson Xavier NX Dev Kit
* Raspberry Pi camera (alternative streaming webcam)
* Robotic Arm (https://github.com/mak3r/edge-robot-demo)
* Rancher Management Server (running where?)

### Stretch Goals

* Build a secondary library to do predictive gameplay for the computer player
* Receive camera feed from webcam interfaces remotely
* Akri integration for camera, robot, jetson

## Demo
The “demo” is broken into two basic parts.
Part 1 is an interactive game of Rock Paper Scissors in which human players attempt to win against the computer.
Part 2 will be a presentation that includes both gameplay and the unpacking of NUCs prepared for the OT environment and dropped in to demonstrate how AI/ML, Edge systems and Secure Device Onboarding can be integrated with cloud native technologies to provide complete, secure and remotely manageable end-to-end solutions.
 
The interactive demo will have 5-6 physical components on the desktop. These include:

- 3 NUCs as the Kubernetes cluster
- A web cam for input of human player gestures
- A robotic arm for expression of the computer gestures
- A device to start/stop/reset the game



