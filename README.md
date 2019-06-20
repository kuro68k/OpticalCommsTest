# OpticalCommsTest
Communicate with an MCU optically from Javascript/web browser

One way communication between Javascript running in a browser and a microcontroller. This is just a prototype/experiment.

An area of the screen is flashed black/white, with data encoded via one of two methods I am experimenting with: pulse width and RS-232/UART. A simple photodiode is used to detect the colour of the screen. A typical configuration would have the diode connected to a comparator. One input would be the signal directly, the other would be a low pass filtered version with slight offset allowing for automatic gain setting. A lower cost option might remove the AGC and replce with a trim pot.

The Javascript code uses frame sync to transmit data as fast as the screen refresh will allow, although that is still relatively slow. With pulse-width encoding the average data rate is around 25 bps on a common 60Hz display. With RS-232/UART encoding it's 48 bps (8n1).

In either case auto-bauding is required. The screen refresh rate can vary, and my goal is to support microcontrollers running from a low accuracy RC oscillator.
