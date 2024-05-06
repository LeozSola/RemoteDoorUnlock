#include <Stepper.h>

const int stepsPerRevolution = 2048;

//Pin 8-11 to IN1-IN4
Stepper stepperName = Stepper(stepsPerRevolution, 8, 10, 9, 11);

const int clockwiseButtonPin = 2;
const int cClockwiseButtonPin = 4;

int clockwiseButtonState = 0;
int cClockwiseButtonState = 0;

void setup() {

  //Set the RPM of the stepper motor
  stepperName.setSpeed(10);
  Serial.begin(9600);

  pinMode(clockwiseButtonPin, INPUT);
  pinMode(cClockwiseButtonPin, INPUT);

}

void loop() {

  // Manual controls
  clockwiseButtonState = digitalRead(clockwiseButtonPin);
  cClockwiseButtonState = digitalRead(cClockwiseButtonPin);

  if (clockwiseButtonState == HIGH) {
    Serial.println("ClockwiseButtonState command received");
    stepperName.step(32);
  }

  if (cClockwiseButtonState == HIGH) {
    Serial.println("CClockwiseButtonState command received");
    stepperName.step(-32);
  }

  if (Serial.available() > 0) {  // Check if there is data waiting in the serial buffer
    String command = Serial.readStringUntil('\n');  // Read until newline character
    command.trim();

    if (command == "unlock") {
        Serial.println("Unlock command received");
        stepperName.step(stepsPerRevolution/2);
        delay(500);
    }

    if (command == "lock") {
        Serial.println("Lock command received");
        stepperName.step(-stepsPerRevolution/2);
        delay(500);
    }

    while (Serial.available() > 0) {  // Clear the buffer
        Serial.read();
    }
  }
    
}