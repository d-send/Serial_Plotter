# Serial_Plotter
This is a real time serial plotter made for Windows for displaying/plotting serial data coming from serial port. Application uses Win32 API and Raylib as the GUI Library. 

![image alt](https://github.com/d-send/Serial_Plotter/blob/8c48e3386291a6e9ac09146a637e511c1e5ebc7e/G3.jpg)

## Features
* Auto Detect the number of channels
* colors of the individual plots can be changed by right clicking on the legend and picking the color from the color palette
* Legend names can be changed by right clicking on the name on the legend of the corresponding plot and renaming
* Manual Scaling can be done by the sliders
* Supports positive and negative integers and floats

## How to send data over Serial
you can use the typical Arduino style serial printing to send data.use a comma delimiter to seperate data channels. 
```c
Serial.print(a);
Serial.print(",");
Serial.println(b);
```
## Watch the video on how i made this
[![Alt text](https://img.youtube.com/vi/Tqb1qtOlFWQ/0.jpg)](https://www.youtube.com/watch?v=Tqb1qtOlFWQ)
