# MIDI Recorder Firmware + CAD

My MIDI recorder is a small device that lives plugged into a MIDI instrument (specifically a digital piano) and automatically records and uploads everything played on it for later reference. The core idea is that no input is required, and no feedback is given that recording has started/stopped, so you get 'recording-by-default'. A webserver automatically processes recordings and makes them available for playback via a frontend website.

---

This is the firmware and CAD files for a device that I'm building that automatically records everything I play on my piano. The repository for the server and frontend website can be found [here](https://github.com/lachlansleight/KeyboardRecorder-Server)

The device is pretty simple - it's an ESP32, some power regulation and MIDI IN / MIDI THRU. Check back soon, I'm actively working on this project right now so CAD files for the enclosure should be coming soon!

![MidiRecorder Circuitboard Render](https://github.com/lachlansleight/KeyboardRecorder-Firmware/raw/main/CAD/PCB_v1_Render.png)

### Bill of Materials

  * ESP32 WROOM (or similar) - note the pinout if you're using my PCB design
  * 6N138 Optocoupler (DIP8)
  * HT7333 Linear Voltage Regulator
  * 1N914 Signal Diode
  * 6 x 220Ω Resistors
  * 4-pin 8mm RGB LED
  * Bidirectional Logic Level Converter (e.g. [this one from sparkfun](https://www.sparkfun.com/products/12009))
  * 2 x 5-pin DIN Jack (panel mount)
  * 1 x DC Barrel Jack (panel mount)
  * 5V 2A DC Power Supply (make sure the barrel size matches your jack size)
