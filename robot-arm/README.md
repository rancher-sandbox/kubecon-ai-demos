# Developer Guide to the Robot Arm 
The robot arm is used to give a physical expression of the computer move in the Rock Paper Scissor Game. The Robot Arm is based on the [edge robot demo](https://github.com/mak3r/edge-robot-demo). 

## Requirements / Dependencies
* Install [pyftdi](https://github.com/eblot/pyftdi) on the linux host
* Build the [Inmoov right arm](http://inmoov.fr/hand-and-forarm/)
* Build a robot arm controller (currently undocumented)
    * Arduino UNO
    * 16x12 bit PWM shield from adafruit
    * FTDI 232H from adafruit
    * Pico PSU
* Install the arduino rps code to the robot arm controller

## Contents

* [arduino](./arduino)
    * [finger-stress-test](./arduino/finger-stress-test) - used to stress the tendons on the inmoov finger test platform
    * [rps](./arduino/rps) - code to run the rock paper scissor game
