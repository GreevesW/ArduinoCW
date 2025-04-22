#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// LCD pin setup
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Input from sensor
SoftwareSerial receiverA(6, 255);  // RX from Transmitter A on pin 6
SoftwareSerial receiverB(7, 255);  // RX from Transmitter B on pin 7

float lightIntensityA = 0;
float temperatureA = 0;
float humidityA = 0;

float lightIntensityB = 0;
float temperatureB = 0;
float humidityB = 0;

float avgLightIntensity = 0;
float avgTemperature = 0;
float avgHumidity = 0;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000; // 2 seconds

void setup() {
  lcd.begin(16, 2); // 16x2 LCD
  Serial.begin(9600);

  receiverA.begin(9600);
  receiverB.begin(9600);

  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
}

void parseData(String dataA, String dataB) {
  // ---------- Parse A ----------
  dataA.trim();
  int firstCommaA = dataA.indexOf(',');
  int secondCommaA = dataA.indexOf(',', firstCommaA + 1);

  if (dataA.indexOf(':') != -1 && firstCommaA != -1 && secondCommaA != -1) {
    String LI_string = dataA.substring(dataA.indexOf(':') + 1, firstCommaA);
    String T_string = dataA.substring(firstCommaA + 1, secondCommaA);
    String H_string = dataA.substring(secondCommaA + 1);

    lightIntensityA = LI_string.toFloat();
    temperatureA = T_string.toFloat();
    humidityA = H_string.toFloat();

    Serial.println("A:");
    Serial.print("Light Intensity: "); Serial.println(lightIntensityA);
    Serial.print("Temperature: "); Serial.print(temperatureA); Serial.println(" C");
    Serial.print("Humidity: "); Serial.print(humidityA); Serial.println(" %");
  }

  // ---------- Parse B ----------
  dataB.trim();
  int firstCommaB = dataB.indexOf(',');
  int secondCommaB = dataB.indexOf(',', firstCommaB + 1);

  if (dataB.indexOf(':') != -1 && firstCommaB != -1 && secondCommaB != -1) {
    String LI_string = dataB.substring(dataB.indexOf(':') + 1, firstCommaB);
    String T_string = dataB.substring(firstCommaB + 1, secondCommaB);
    String H_string = dataB.substring(secondCommaB + 1);

    lightIntensityB = LI_string.toFloat();
    temperatureB = T_string.toFloat();
    humidityB = H_string.toFloat();

    Serial.println("B:");
    Serial.print("Light Intensity: "); Serial.println(lightIntensityB);
    Serial.print("Temperature: "); Serial.print(temperatureB); Serial.println(" C");
    Serial.print("Humidity: "); Serial.print(humidityB); Serial.println(" %");
  }
}


void getValues() {
  String latestLineA = "";
  String latestLineB = "";
  
  // Listen to receiver A
  receiverA.listen();
  unsigned long startTime = millis();
  // Give it time to receive data (max 500ms)
  while (millis() - startTime < 2000) {
    if (receiverA.available()) {
      latestLineA = receiverA.readStringUntil('\n');
      if (latestLineA.length() > 0) break;
    }
  }
  
  // Listen to receiver B
  receiverB.listen();
  startTime = millis();
  // Give it time to receive data (max 500ms)
  while (millis() - startTime < 2000) {
    if (receiverB.available()) {
      latestLineB = receiverB.readStringUntil('\n');
      if (latestLineB.length() > 0) break;
    }
  }
  
  // Process any received data
  if (latestLineA.length() > 0 || latestLineB.length() > 0) {
    Serial.println("---- New Data ----");
    Serial.print("Latest from A: ");
    Serial.println(latestLineA);
    Serial.print("Latest from B: ");
    Serial.println(latestLineB);
    parseData(latestLineA, latestLineB);
  }

  avgTemperature = (temperatureA + temperatureB) / 2;
  avgHumidity = (humidityA + humidityB) / 2;
  avgLightIntensity = (lightIntensityA + lightIntensityB)/2;

}


void printText() {
  digitalWrite(10, LOW);
  digitalWrite(9, LOW);
  digitalWrite(8, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(avgTemperature, 1);  // 1 decimal place
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Hum:");
  lcd.print(avgHumidity, 1);     // 1 decimal place
  lcd.print("%");

  lcd.setCursor(10, 1);
  lcd.print("LI:");
  lcd.print(avgLightIntensity, 0); // No decimals for light intensity

  float eqi = calculateEQI(avgTemperature, avgHumidity, avgLightIntensity);
  Serial.print("EQI: ");
  Serial.println(eqi, 2);
  lcd.setCursor(11, 0); 
  if (eqi >= 0.8){
    lcd.print("Good");
    digitalWrite(10, HIGH);
  }
  else if (eqi >= 0.5){
    lcd.print("Med ");
    digitalWrite(9, HIGH);
  }
  else{
    lcd.print("Poor");
    digitalWrite(8, HIGH);
  }
}

float scoreTemperature(float temp) {
  if (temp >= 20 && temp <= 25) return 1.0;
  else if ((temp >= 18 && temp < 20) || (temp > 25 && temp <= 28)) return 0.5;
  else return 0.0;
}

float scoreHumidity(float hum) {
  if (hum >= 40 && hum <= 60) return 1.0;
  else if ((hum >= 30 && hum < 40) || (hum > 60 && hum <= 70)) return 0.5;
  else return 0.0;
}

float scoreLight(float light) {
  if (light >= 300 && light <= 500) return 1.0;
  else if ((light >= 200 && light < 300) || (light > 500 && light <= 700)) return 0.5;
  else return 0.0;
}

float calculateEQI(float temp, float hum, float light) {
  float tempScore = scoreTemperature(temp);
  float humScore = scoreHumidity(hum);
  float lightScore = scoreLight(light);

  // Assign weights (total = 1.0)
  float EQI = (0.4 * tempScore) + (0.3 * humScore) + (0.3 * lightScore);
  return EQI;
}


void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate >= updateInterval) {
    lastUpdate = currentMillis;
    getValues();
    printText();
  }
}
