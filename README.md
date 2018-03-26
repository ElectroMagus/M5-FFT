This is a basic FFT to create a graphic equalizer of audio data received by microphone (MAX9814 in this case).   I wrote it to explore the capabilities of the ESP32 Analog to Digital Converter as well as the very nice M5Stack platform.

It uses the basic Arduino FFT library which was designed for 8 bit microcontrollers making it very efficient. The graphics were adapted from various sources to work on the M5Stack display.  Future enhancements will utilize the hardware capabilities of the M5Stack platform and the extra oomph from the ESP32 platform.

Feel free to use, adapt or improve the code.  Feel free to send those changes back in a pull request as well.




M5Stack Information
It's powered by an ESP32 and is tightly integrated with the following hardware:
- Modular design to allow for custom hardware additions
- 3 Buttons
- 320x240 LCD (ILI9341)
- SD Card Socket
- Speaker (NS4148)
- 150 mAh battery with intelligent IP5306 charger
- Grove I2C Interface
- Exposed male/female headers in the case for access to GPIO/SPI/I2C/I2S/DAC/ADC
