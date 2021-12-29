# Naschkatzen_Matrix Embedded Systems Project
## Overview
The main objective is the development of a wireless controlled lighting matrix for the A-building of the Sontheim campus in Heilbronn, Germany. In addition, this project aims to simplify the process of creating such a matrix by being coded modular and adaptable. Below is a simplified overview of the backend stack:

![Overview](Images/overview.jpg)

We are using the ![PixelBridge](https://github.com/LeoDJ/PixelBridge) as our input. Here a webinterface or simple artnet packages can be used to control the matrix. The PixelBridge is then providing the MatrixBridge with ![Open Pixel Control](http://openpixelcontrol.org/) packages. This Bridge is then using ![painlessMesh](https://gitlab.com/painlessMesh/painlessMesh) to connected embedded devices, in our case ESP32s.

### ESP32 Overview
The ESP32s are using the painlessMesh library to connect to the mesh and do mesh things. Each ESP can have a number of connected LED Strips which are driven via the ![FastLED](https://github.com/FastLED/FastLED) library. For optimizing costs and keeping the installation stealthy we chose simple 5050 rgb strips and created a driver board with a WS2811 IC.

![WindowController PCB](https://github.com/Boslx/Naschkatzen_Matrix/blob/newOrder/WindowController/PCB_WindowController.png)

Each window is represented as one WS2811 pixel. The ESP communicates to the boards via a daisy-chained RS485 Bus.

## Building
### Dependencies
