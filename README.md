Optiboot_Over_UART
One Paragraph of project description goes here
Getting Started
This sketch will allow programming of an Atmega 328 or similar that has optiboot installed, over serial UART. Currently set up for Teensy 3.6 to proigram a 328AU over Serial port 2 (Teensy 3.6 pins 9 and 10) connected to the Serial UART on the 328 directly.
Prerequisites
Teensy 3.6 or any other arduino compatible board with UART. 

ATMEGA 328  - ideally running as 328 on breadboard internal 8MHz clock. Optboot should be installed on this. 

Installing
Connect the Serial2 port of the Teensy to the UART on the 328. 
Programmer – Teensy 3.6
Target – 328 AU with Optiboot
10 –TX2
PD0 – Pin 30 – Arduino Pin 0 RX
09 – RX2
PD1 – Pin 31 – Arduino Pin 0 TX

Running the tests
None yet.
Deployment
To be deployed as a live updater as part of firmware for in situ modules. 
Authors
      To be added
Acknowledgments
      
    • Limor Fried / Ladyada / Adafruit
    • Optiloader  -  Bill Westfield ("WestfW")
