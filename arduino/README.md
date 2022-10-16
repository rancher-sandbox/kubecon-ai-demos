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
    * [finger-stress-test](./finger-stress-test) - used to stress the tendons on the inmoov finger test platform
    * [robot-control]](./robot-control) - code to run the rock paper scissor game
    * [i2c-setup](./i2c-setup) - code used to check the eeprom content and update it with the desired base configuration
