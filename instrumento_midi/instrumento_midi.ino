/**
* Serial output device, designed for a 4x4 button matrix.
* Row pins are outputs, column pins are inputs.
* Scans each row by setting it to high, then picks up the output of each column,
* such that if the output of the column is high, then the button located in that
* row, in that column, is pressed.
* Then every time there is a state change (so a button is pressed/unpressed), it outputs two bytes in the serial port,
* representing the button state. Most significant bit is button at row 4, column 4,
* least significant bit is button at row 1, column 1. The first output byte are the 8 most significant
* bits, the second output byte are the 8 least significant bits.
*/

const int rows = 4;
const int columns = 4;

int rowPins[] = {2, 3, 4, 5};
int colPins[] = {11, 10, 9, 8};

int matrix[rows][columns];
uint16_t prevButtonStates = 0;

unsigned long previousMillis = 0;

void setup() {
  for (int i = 0; i < rows; i++) {
    pinMode(rowPins[i], OUTPUT);
  }

  for (int i = 0; i < columns; i++) {
    pinMode(colPins[i], INPUT);
  }

  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long currentInterval = currentMillis - previousMillis;

  if (currentInterval < 100) return;

  uint16_t buttonStates = 0;
  for (int i = 0; i < rows; i++) {
    digitalWrite(rowPins[i], HIGH);
    for (int j = 0; j < columns; j++) {
      int val = digitalRead(colPins[j]);
      int button = i + j * 4;
      if (val == HIGH) {
        matrix[i][j] = 1;
        buttonStates = buttonStates | (1 << button);
      } else {
        matrix[i][j] = 0;
        buttonStates = buttonStates & ~(1 << button);
      }
    }
    digitalWrite(rowPins[i], LOW);
  }

  int potRead = analogRead(A0);
  uint8_t octave = potRead / 100;

  if (buttonStates != prevButtonStates) {
    uint8_t upperBits = (buttonStates & 0xFF00) >> 8;
    uint8_t lowerBits = buttonStates & 0x00FF;
    Serial.write(0xFF);
    Serial.write(upperBits);
    Serial.write(lowerBits);
    Serial.write(octave);
    Serial.write(0x00);
    prevButtonStates = buttonStates;
  }
}
