#include <Arduino.h>
#include <HID-Project.h>
#include <Bounce2.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

const uint8_t RXLED = 15;
const uint8_t SWITCH_PROGRAM = 6;
const uint8_t BUTTON_MUTE = 5;

Button program = Button();
Bounce mute = Bounce();
boolean hidEnabled = false;

ClickEncoder encoder(A1, A0);
int16_t current, value;

void timerIsr() {
  encoder.service();
}

void enableHid() {
  Consumer.begin();
  Serial.println("enableHid");
  hidEnabled = true;
  digitalWrite(RXLED, HIGH);
}

void disableHid() {
  Consumer.end();
  Serial.println("disableHid");
  hidEnabled = false;
  digitalWrite(RXLED, LOW);
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
    if (program.pressed() && !hidEnabled) {
      enableHid();
      // toggleMute();
  } else if (program.released() && hidEnabled) {
      disableHid();
  }
}

void handleVolume() {
  value += encoder.getValue();
  if (value > current) {
      Consumer.write(MEDIA_VOL_UP);
      Serial.print(value);
      Serial.println(" volDown");
      current = value;
  } else if (value < current) {
      Consumer.write(MEDIA_VOL_DOWN);
      Serial.print(value);
      Serial.println(" volUp");
      current = value;
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
  program.interval(25);
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
