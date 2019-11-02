
const int ledPin =  13;      // the number of the LED pin
int buttonPins[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }; 
int buttonCount = 11;

int buttonState = LOW;         // variable for reading the pushbutton status
int activePin = -1;

void setup() {
  Serial.begin(9600);
  
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  
  // initialize the pushbutton pins as inputs: 
  for (int thisPin = 0; thisPin < buttonCount; thisPin++) {
    pinMode(buttonPins[thisPin], INPUT_PULLUP);
  }
  
  Serial.println();
  Serial.print("*** Test Ready : LED should be off ***");
  Serial.println();
}

void loop() {

  buttonState = LOW;
  
  if (digitalRead(buttonPins[activePin])) { activePin = -1;} //reset the pin when button is released
  
  for (int thisPin = 0; thisPin < buttonCount; thisPin++) {
 
    if (! digitalRead(buttonPins[thisPin])) {
      if (activePin != thisPin) {
        
        Serial.print("PIN ");
        Serial.print(buttonPins[thisPin]);
        Serial.println( " Activated");
        
        flashLED(buttonPins[thisPin]);
        activePin = thisPin;
      
      }
    }
  }
}

void flashLED(int count) {

  for (int i = 0; i < count; i++) {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(300);              // wait for a second
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    delay(500);              // wait for a second
  }
}
