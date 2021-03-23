struct Message {
    byte onOffPitch;
    byte isPedalVelocity;
    byte timeA;
    byte timeB;
    byte timeFrac;
};


#include <WiFi.h>
#include <HTTPClient.h>
const char* ssid     = "WhatFight4";
const char* password = "dumplingsatnoon!";
String server = "http://192.168.20.12:3000"; //for development
String path = "/api/upload";

#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

//RUNTIME VARS
unsigned long songStartTime = 0;
unsigned long songTime = 0;
unsigned long lastNoteTime = 0;
bool runningSong = false;

byte messageBuffer[20000];
unsigned int messageIndex = 0;
unsigned int messageCount = 0;


//CONFIG
//long timeoutTime = 60000; //60 seconds silence = end song
long timeoutTime = 5000; //5 seconds silence = end song (!!for testing!!)

void setup() {
    Serial.begin(38400);
    delay(10);

    //connect to WiFi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");

    //WIFI is now connected, we can start listening for MIDI
    
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleControlChange(handleControlChange);
    MIDI.begin();
}

void loop() {
    //song auto-ends if there's been a certain amount of time passed with no new messages
    //todo - this should probably suspend if there are notes held. Maybe.
    if(runningSong) {
        songTime = (unsigned long)(millis() - songStartTime);
        if(songTime - lastNoteTime > timeoutTime) {
          endSong();
        }
    }
    MIDI.read();
}

void printByte(byte in) {
    byte i = 7;
    while(i >= 0 && i < 8) {
        Serial.print(bitRead(in, i));
        i--;
    }
}

//creates five message bytes from the basic data
Message createMessage(bool onOff, byte pitch, byte velocity, bool isPedal, long songMillis) {
    //time is stored in three bytes - two for the number of seconds, and one for the number of milliseconds (with 4ms precision)
    float songSeconds = (float)songMillis / 1000.0;
    unsigned int wholeSeconds = (unsigned int)songSeconds;
    unsigned int fracSeconds = (unsigned int)(songMillis % 1000);
    fracSeconds /= 4;

    //first byte contains on/off status, and pitch
    byte firstByte = onOff << 7 | (pitch & 0b01111111);
    //second byte contains whether this message is a pedal movement or not, and velocity
    byte secondByte = isPedal << 7 | (velocity & 0b01111111);
    //time bytes, as mentioned above
    byte thirdByte = wholeSeconds >> 8;
    byte fourthByte = wholeSeconds & 0b0000000011111111;
    byte fifthByte = (byte)fracSeconds;

    Message newMessage{
        firstByte,
        secondByte,
        thirdByte,
        fourthByte,
        fifthByte
    };
    printMessage(newMessage);
    Serial.println("\t" + String(onOff) + "\t" + String(pitch) + "\t" + String(velocity) + "\t    " + String(isPedal) + "\t" + String(songMillis));
    Serial.println("");
    return newMessage;
}

//creates a message from the basic data and adds the five resulting bytes to the buffer
//also starts a new song if necessary, and updates the last played note time for calculating timeout
void addMessage(bool onOff, byte pitch, byte velocity, bool isPedal, long songMillis) {
    if(!runningSong) startSong();
    
    Message newMessage = createMessage(onOff, pitch, velocity, isPedal, songMillis);
    messageBuffer[messageIndex] = newMessage.onOffPitch;
    messageIndex++;
    messageBuffer[messageIndex] = newMessage.isPedalVelocity;
    messageIndex++;
    messageBuffer[messageIndex] = newMessage.timeA;
    messageIndex++;
    messageBuffer[messageIndex] = newMessage.timeB;
    messageIndex++;
    messageBuffer[messageIndex] = newMessage.timeFrac;
    messageIndex++;
    
    messageCount++;
    lastNoteTime = songMillis;
}

void printMessage(Message message) {
    Serial.print("  ");
    printByte(message.onOffPitch);
    Serial.print("  ");
    printByte(message.isPedalVelocity);
    Serial.print("  ");
    printByte(message.timeA);
    Serial.print("  ");
    printByte(message.timeB);
    Serial.print("  ");
    printByte(message.timeFrac);
    Serial.println("");
}

//uploads the current song buffer to the server to be further processed
void uploadSong()
{
    if(WiFi.status()== WL_CONNECTED){
        HTTPClient http;
        
        Serial.println("Making request to " + (server + path));
        http.begin((server + path).c_str());
        
        //data
        http.addHeader("Content-Type", "application/octet-stream");
        int httpResponseCode = http.POST(messageBuffer, messageCount * 5);
        
        if (httpResponseCode>0) {
            String payload = http.getString();
            Serial.print("HTTP Response: ");
            Serial.print(httpResponseCode);
            Serial.print(" - ");
            Serial.println(payload);
        }
        else {
            if(httpResponseCode == -1) Serial.println("Error - Connection_Refused (Code -1)");
            else if(httpResponseCode == -2) Serial.println("Error - Send Header Failed (Code -2)");
            else if(httpResponseCode == -3) Serial.println("Error - Send Payload Failed (Code -3)");
            else if(httpResponseCode == -4) Serial.println("Error - Not Connected (Code -4)");
            else if(httpResponseCode == -5) Serial.println("Error - Connection Lost (Code -5)");
            else if(httpResponseCode == -6) Serial.println("Error - No Stream (Code -6)");
            else if(httpResponseCode == -7) Serial.println("Error - No Http Server (Code -7)");
            else if(httpResponseCode == -8) Serial.println("Error - Not Enough RAM (Code -8)");
            else if(httpResponseCode == -9) Serial.println("Error - Encoding (Code -9)");
            else if(httpResponseCode == -10) Serial.println("Error - Stream Write (Code -10)");
            else if(httpResponseCode == -11) Serial.println("Error - Read Timeout (Code -11)");
            else Serial.println("Error - Unexpected error code " + String(httpResponseCode));
        }
        // Free resources
        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }
}

void startSong()
{
    Serial.println("====================STARTING SONG===================");
    songStartTime = millis();
    runningSong = true;
}

//resets everything for a new song, and uploads the song if there have been messages entered
void endSong()
{
    if(messageCount > 0) {
        uploadSong();
    }
    
    Serial.println("====================ENDING SONG===================");
    messageIndex = 0;
    messageCount = 0;
    runningSong = false;
}





void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    //Serial.println("ON\t" + String(channel) + "\t" + String(pitch) + "\t" + String(velocity));
    addMessage(true, pitch, velocity, false, songTime);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    //Serial.println("OFF\t" + String(channel) + "\t" + String(pitch) + "\t" + String(velocity));
    addMessage(false, pitch, velocity, false, songTime);
}

void handleControlChange(byte channel, byte number, byte value)
{
    //Serial.println("CC\t" + String(channel) + "\t" + String(number) + "\t" + String(value));
    if(number == 64) {
        //dampen pedal
        if(value == 127) addMessage(true, 0, 0, true, songTime);
        else addMessage(true, 0, 0, false, songTime);
    } else if(number == 67) {
        //sost pedal - could use for some kind of input?
        //I'm thinking a hold of three seconds on sost should discard the current song without uploading
    } else {
        //unknown / uncared-about
    }
}
