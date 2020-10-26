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
int16_t current, value;

void timerIsr() {
    encoder.service();
}

void enableHid() {
    Consumer.begin();
    Serial.println("enableHid");
    hidEnabled = true;
    digitalWrite(RXLED, LOW);
}

void disableHid() {
    Consumer.end();
    Serial.println("disableHid");
    hidEnabled = false;
    digitalWrite(RXLED, HIGH);
}

void toggleMute() {
    Serial.println("toggleMute");
    Consumer.write(MEDIA_VOLUME_MUTE);
}

void handleMute() {
    mute.update();
    if (mute.fell()) {
        toggleMute();
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
    current += encoder.getValue();
    auto diff = current - value;
    if (diff < 0) {
        Consumer.write(MEDIA_VOL_UP);
        Serial.print(value);
        Serial.println(" volDown");
        value = current;
    } else if (diff > 0) {
        Consumer.write(MEDIA_VOL_DOWN);
        Serial.print(value);
        Serial.println(" volUp");
        value = current;
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(RXLED, OUTPUT);
    digitalWrite(RXLED, LOW);

    pinMode(BUTTON_MUTE, INPUT);
    digitalWrite(BUTTON_MUTE, HIGH);
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
