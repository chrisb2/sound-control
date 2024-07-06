#include <Arduino.h>
#include <HID-Project.h>
#include <Bounce2.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

const uint8_t RXLED = 5;
const uint8_t SWITCH_PROGRAM = 3;
const uint8_t BUTTON_MUTE = 15;

Button program = Button();
Bounce mute = Bounce();
boolean hidEnabled = true;

ClickEncoder encoder(A0, A1);
int16_t newVolume, existingVolume;

void timerIsr() {
    encoder.service();
}

void enableHid() {
    Consumer.begin();
    Keyboard.begin();
    Serial.println("enableHid");
    hidEnabled = true;
    digitalWrite(RXLED, LOW);
}

void disableHid() {
    Consumer.end();
    Keyboard.end();
    Serial.println("disableHid");
    hidEnabled = false;
    digitalWrite(RXLED, HIGH);
}

void toggleMuteSpeaker() {
    Serial.println("toggleMuteSpeaker");
    Consumer.write(MEDIA_VOLUME_MUTE);
}

void toggleMuteMicrophone() {
    /*
       The USB HIB specification does not provide the ability to mute/unmute the
       microphone, so this functionality relies on the installation of a utility
       program on the PC such as MicMute (https://sourceforge.net/projects/micmute/)
       to listen for a key combination and mute/unmute the microphone, or key
       combinations supported by specific applications.
     */
    Serial.println("toggleMuteMicrophone");
    // Microsoft Outlook Meeting
    Keyboard.press(KEY_RIGHT_CTRL);
    Keyboard.press(KEY_RIGHT_SHIFT);
    Keyboard.write(KEY_M);
    Keyboard.release(KEY_RIGHT_SHIFT);
    Keyboard.release(KEY_RIGHT_CTRL);
}

void handleMute() {
    mute.update();
    if (mute.fell()) {
        toggleMuteMicrophone();
    }
}

void handleProgram() {
    program.update();
    if (program.released() && !hidEnabled) {
        enableHid();
    } else if (program.pressed() && hidEnabled) {
        disableHid();
    }
}

void handleVolume() {
    newVolume += encoder.getValue();
    auto diff = newVolume - existingVolume;
    if (diff < 0) {
        Consumer.write(MEDIA_VOL_UP);
        Serial.print(newVolume);
        Serial.println(" volDown");
        existingVolume = newVolume;
    } else if (diff > 0) {
        Consumer.write(MEDIA_VOL_DOWN);
        Serial.print(newVolume);
        Serial.println(" volUp");
        existingVolume = newVolume;
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(RXLED, OUTPUT);
    digitalWrite(RXLED, LOW);

    mute.attach(BUTTON_MUTE, INPUT_PULLUP);
    mute.interval(5);

    program.attach(SWITCH_PROGRAM, INPUT_PULLUP);
    program.interval(100);
    program.setPressedState(LOW);

    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);
}

void loop() {
    handleProgram();
    if (hidEnabled) {
        handleMute();
        handleVolume();
    }
}
