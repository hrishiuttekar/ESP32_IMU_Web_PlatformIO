/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp32-mpu-6050-web-server/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include "JarzebskiMPU6050/MPU6050.h"
#include <Arduino_JSON.h>
#include "LittleFS.h"

// Network credentials live in src/secrets.h (not committed).
// Copy src/secrets.example.h to src/secrets.h and fill in your Wi-Fi details.
#include "secrets.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;  
unsigned long lastTimeTemperature = 0;
unsigned long lastTimeAcc = 0;
unsigned long gyroDelay = 10;
unsigned long temperatureDelay = 1000;
unsigned long accelerometerDelay = 200;

// Create a sensor object
constexpr int SDA_PIN = 21;
constexpr int SCL_PIN = 22;
constexpr uint8_t MPU_ADDRESS = 0x68;
MPU6050 mpu;
bool serverReady = false;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float temperature;

//Gyroscope sensor deviation
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;

bool initMPU() {
  if (!Wire.setPins(SDA_PIN, SCL_PIN) || !Wire.begin()) {
    Serial.println("Could not start I2C.");
    return false;
  }

  constexpr uint8_t WHO_AM_I = 0x75;
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(WHO_AM_I);
  if (Wire.endTransmission(false) != 0 ||
      Wire.requestFrom(MPU_ADDRESS, size_t{1}) != 1) {
    Serial.println("Could not read the IMU. Check SDA, SCL, power, and AD0.");
    return false;
  }

  const int id = Wire.read();
  Serial.printf("I2C address 0x%02X, WHO_AM_I = 0x%02X\n",
                static_cast<unsigned>(MPU_ADDRESS), static_cast<unsigned>(id));
  if ((id != 0x70 && id != 0x68) ||
      !mpu.begin(MPU6050_SCALE_250DPS, MPU6050_RANGE_2G, MPU_ADDRESS)) {
    Serial.println("IMU initialization failed. Expected an MPU-6500 or MPU-6050.");
    return false;
  }

  Serial.println("Keep the sensor still while the gyro calibrates.");
  delay(1000);
  mpu.calibrateGyro();
  return true;
}

bool initLittleFS() {
  if (!LittleFS.begin() || !LittleFS.exists("/index.html") ||
      !LittleFS.exists("/style.css") || !LittleFS.exists("/script.js")) {
    Serial.println("LittleFS files are missing. Upload the data folder as a LittleFS image.");
    return false;
  }
  Serial.println("LittleFS mounted successfully.");
  return true;
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
}

String getGyroReadings(){
  const Vector gyro = mpu.readNormalizeGyro();

  float gyroX_temp = gyro.XAxis * DEG_TO_RAD;
  if(abs(gyroX_temp) > gyroXerror)  {
    gyroX += gyroX_temp/50.00;
  }
  
  float gyroY_temp = gyro.YAxis * DEG_TO_RAD;
  if(abs(gyroY_temp) > gyroYerror) {
    gyroY += gyroY_temp/70.00;
  }

  float gyroZ_temp = gyro.ZAxis * DEG_TO_RAD;
  if(abs(gyroZ_temp) > gyroZerror) {
    gyroZ += gyroZ_temp/90.00;
  }

  readings["gyroX"] = String(gyroX);
  readings["gyroY"] = String(gyroY);
  readings["gyroZ"] = String(gyroZ);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

String getAccReadings() {
  const Vector acceleration = mpu.readNormalizeAccel();
  accX = acceleration.XAxis;
  accY = acceleration.YAxis;
  accZ = acceleration.ZAxis;
  readings["accX"] = String(accX);
  readings["accY"] = String(accY);
  readings["accZ"] = String(accZ);
  String accString = JSON.stringify (readings);
  return accString;
}

String getTemperature(){
  temperature = mpu.readTemperature();
  return String(temperature);
}

void setup() {
  Serial.begin(115200);
  if (!initLittleFS() || !initMPU()) {
    return;
  }
  initWiFi();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroX=0;
    gyroY=0;
    gyroZ=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroX=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroY=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroZ=0;
    request->send(200, "text/plain", "OK");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  server.begin();
  serverReady = true;
}

void loop() {
  if (!serverReady) {
    delay(1000);
    return;
  }

  if ((millis() - lastTime) > gyroDelay) {
    // Send Events to the Web Server with the Sensor Readings
    events.send(getGyroReadings().c_str(),"gyro_readings",millis());
    lastTime = millis();
  }
  if ((millis() - lastTimeAcc) > accelerometerDelay) {
    // Send Events to the Web Server with the Sensor Readings
    events.send(getAccReadings().c_str(),"accelerometer_readings",millis());
    lastTimeAcc = millis();
  }
  if ((millis() - lastTimeTemperature) > temperatureDelay) {
    // Send Events to the Web Server with the Sensor Readings
    events.send(getTemperature().c_str(),"temperature_reading",millis());
    lastTimeTemperature = millis();
  }
}