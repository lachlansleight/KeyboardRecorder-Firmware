#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

void setup() {
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  Serial.begin(38400);

  Serial.println("MIDI TEST BEGIN");

  MIDI.begin();
}

void loop() {
  MIDI.read();
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    Serial.println("ON\t" + String(channel) + "\t" + String(pitch) + "\t" + String(velocity));
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    Serial.println("OFF\t" + String(channel) + "\t" + String(pitch) + "\t" + String(velocity));
}

void handleControlChange(byte channel, byte number, byte value)
{
    Serial.println("CC\t" + String(channel) + "\t" + String(number) + "\t" + String(value));
}
