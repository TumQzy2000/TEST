#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <HTTPClient.h>
#include "Adafruit_SGP30.h"

BlynkTimer timer;
int mode = 0;  // 0 = Auto, 1 = Manual
char auth[] = "DKZIQq0hcRazIPcIqwwS7lzLnlRi_H06";
char ssid[] = "Projectstartup"; 
char pass[] = "GOGOStartup24";  

#define relay1 25  
WidgetLED LED1 (V9)


Adafruit_SGP30 sgp;

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

int counter = 0;
void sendSensor()
{
  //SGP30/
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
  Blynk.virtualWrite(V8, sgp.eCO2);
  
  counter++;
  if (counter == 30) {
      counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }
  

  // Check WiFi connection before sending data
  if (WiFi.status() == WL_CONNECTED) {
    // Create HTTP client object
    HTTPClient http;

    // Construct URL with sensor value
    String url = "https://script.google.com/macros/s/AKfycbzrS3rZc9Xg8BCL3uuKH3dgZYPy43YXQKCVT8AZf8UhsbDpPjyEzBoNPh4ibHJhPB-1fw/exec?value="+String(sgp.eCO2);

    // Start HTTP request
    Serial.println("Making a request");
    http.begin(url.c_str());

    // Set HTTP request options
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // Send GET request and get response code
    int httpCode = http.GET();

    // Check response code
    String payload;
    if (httpCode > 0) {
      // Read response payload
      payload = http.getString();

      // Print response code and payload to serial monitor
      Serial.println(httpCode);
      Serial.println(payload);
    } else {
      // Handle error if response code is not positive
      Serial.println("Error on HTTP request");
    }

    // Close HTTP connection
    http.end();
  } else {
    // Handle situation when Wi-Fi is not connected
    Serial.println("Wi-Fi not connected, skipping data transmission");
  }

  //Delay between transmissions
  //delay(5000);
}



void setup()
{
  Serial.begin(115200);
  pinMode(relay1, OUTPUT); //ปั๊มน้ำ
  digitalWrite(relay1, LOW);
  
 // Initialize Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize Blynk
  Blynk.begin(auth, ssid, pass, "blynk.iot-cm.com", 8080);

  //Initialize SGP30 sensor
    Serial.println("SGP30 test");
  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
}


BLYNK_WRITE(V2)
{
  int pinValue = param.asInt();  
  if (pinValue == 1) {
    digitalWrite(relay1, HIGH); 
    Serial.println("Relay is ON");
    LED1.on();
  } else {
    digitalWrite(relay1, LOW);   
    Serial.println("Relay is OFF");
     LED1.off();
  }
}

void loop(){
  Blynk.run();
  timer.run();
}
