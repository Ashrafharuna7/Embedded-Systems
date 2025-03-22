# STM32 Fan Speed Controller
## Introduction
The aim of this project is to control the speed of a PC fan using **PWM** on an STM32L432KC microcontroller. 
The fan speed is measured using an in built tachometer.

## Features
- PA3 is used to control the fan speed using its alternative function **PWM** mode.
- PA1 is used to measure the speed of the fan using an in built tachometer with its timer alternative function mode.
- PA0 is used to read the input voltage from the potentiometer and convert it to a digital value. (Analog to digital).
- PA15 and PA2 is used to send the RPM readings of the fan to a serial monitor using **UART**.

## Circuit Diagram
The fan is powered using a 12V DC power supply and has its tachometer and PWM connected to PA1 and PA3 respectively. The microcontroller is connected via USB to the laptop. The poteniometer is connected to the 3.3V of the microcontroller and PA0. This allows the microcontroller to read the input voltage from the potentiometer as a digital value. There is a pullup resistor connected to the tachometer and 3.3V to pull the signal **HIGH** when the fan is not pulling it **LOW**. The microcontroller uses UART serial communication to communicate to the laptop. This is done on both PA2 and PA15. The circuit digram can be seen here:

    
