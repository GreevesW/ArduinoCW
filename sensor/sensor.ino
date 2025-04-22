#include <DHT.h>
#include <DHT_U.h>
#include <SoftwareSerial.h>

#define DHT11_PIN 7

// Define TX pin (3) and a valid RX pin (you can use 255 if you don't need RX)
SoftwareSerial softSerial(255, 3); // (RX, TX) - using 255 means we don't need to receive

DHT dht(DHT11_PIN, DHT11);  // Initialize DHT object

void setup() {
    Serial.begin(9600);
    softSerial.begin(9600);
    dht.begin();  // Initialize DHT sensor
}

void loop() {
    int lightValue = analogRead(A0);
    int tempValue = analogRead(A1);
    
    // Read humidity from DHT sensor
    float humidity = dht.readHumidity();
    String humidityString = String(humidity, 1);
    
    // Read temperature from DHT sensor (more reliable than analog conversion)
    float temperature = dht.readTemperature();  // Temperature in Celsius
    
    // If you still want to use the analog temperature:
    float analogTemp = ((tempValue * (5.0 / 1023.0)) - 0.5) * 100;
    String tempString = String(analogTemp, 1);
    
    // Send data through software serial
    softSerial.print("A:");
    softSerial.print(lightValue);
    softSerial.print(",");
    softSerial.print(tempString); // Using DHT temperature
    softSerial.print(",");
    softSerial.println(humidityString);
    
    // Debug output to serial monitor
    Serial.print("A:");
    Serial.print(lightValue);
    Serial.print(",");
    Serial.print(tempString);
    Serial.print(",");
    Serial.println(humidityString);
    
    delay(2000);
}
