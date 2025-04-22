#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// LCD pin setup
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Input pins from sensor
SoftwareSerial receiverA(6, 255);  // RX from Transmitter A on pin 4
//SoftwareSerial receiverB(9, 255);  // RX from Transmitter B on pin 5

void setup() {
  lcd.begin(16, 2); // 16 columns, 2 rows (screen size)

  Serial.begin(9600);
  receiverA.begin(9600);
  //receiverB.begin(9600);

}

// scrolls the text across the screen
void scrollText(String message, int row) {
  lcd.clear();
  lcd.setCursor(15, row); // Start at the far right
  lcd.autoscroll();

  //prints the string, letter by letter
  for (int i = 0; i < message.length(); i++) {
    lcd.print(message[i]);
    delay(300);
  }

  //pushes the string off of the screen
  for (int i = 0; i < 15; i++) {
    lcd.print(" ");
    delay(100);
  }

  lcd.noAutoscroll();
  // delay(1000);
  lcd.clear();
}

void getValues() {
  String latestLine = "";

  // Keep reading until the buffer is empty, grabbing the last complete line
  while (receiverA.available()) {
    latestLine = receiverA.readStringUntil('\n');
  }

  if (latestLine.length() > 0) {
    Serial.print("Latest from A: ");
    Serial.println(latestLine);
    parseData(latestLine);
  }
}

void parseData(String data) {
  // Remove any newline or carriage return
  data.trim();

  // Expected format: A:0,22.10,51.00
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);

  // Extract light intensity (the part after the colon and before the first comma)
  String lightIntensity = data.substring(data.indexOf(':') + 1, firstComma);

  // Extract temperature (the part between the first and second comma)
  String temperature = data.substring(firstComma + 1, secondComma);

  // Extract humidity (the part after the second comma)
  String humidity = data.substring(secondComma + 1);
  

  // Display the values
  Serial.println("Light Intensity: " + lightIntensity);
  Serial.println("Temperature: " + temperature + " C");
  Serial.println("Humidity: " + humidity + " %");

  scrollText("Light Intensity: " + lightIntensity, 0);
  scrollText("Temperature: " + temperature + " C", 0);
  scrollText("Humidity: " + humidity + " %", 0);

}



String inputBuffer = "";  // Holds partial serial data

void loop() {
  while (receiverA.available()) {
    char incomingChar = receiverA.read();

    // this is needed to prevent communications, where the information is cut short by the scrolling mechanism


    if (incomingChar == '\n') {
      inputBuffer.trim();  // Clean up extra spaces

      //only passes communications with the correct format
      if (inputBuffer.startsWith("A:") && inputBuffer.indexOf(',') != -1) {
        parseData(inputBuffer);
      } else {
        Serial.println("Invalid or malformed data: " + inputBuffer);
      }
      inputBuffer = ""; // Clear buffer after processing
    } else {
      inputBuffer += incomingChar;
    }
  }
}

