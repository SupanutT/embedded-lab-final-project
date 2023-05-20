#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#endif

EspSoftwareSerial::UART testSerial;

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "<your_WIFI_SSID>"
#define WIFI_PASSWORD "<YOUR_WIFI_PASSWORD>"

#define API_KEY "AIzaSyDVomwtqAWlUln7ft-qbkgpWVecurVZrQE"

#define DATABASE_URL "nodemcu-f263e-default-rtdb.firebaseio.com"  

#define USER_EMAIL "admin@admin.com"
#define USER_PASSWORD "thisispassword"

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void setup() {

  Serial.begin(115200);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  multi.addAP(WIFI_SSID, WIFI_PASSWORD);
  multi.run();
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    if (millis() - ms > 10000)
      break;
#endif
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;

  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);
  testSerial.begin(115200, EspSoftwareSerial::SWSERIAL_8N1, D7, D8, false, 95, 11);
  if (!testSerial) {
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("EspSoftwareSerial init");
}

void loop() {
  if (testSerial.available() && Firebase.ready()) {
    int initialData = testSerial.read();
    Serial.println("---------");
    while (initialData != 255) {
      if (testSerial.available()) {
        initialData = testSerial.read();
      }
    }
    Serial.println("initialData OK");

    while (!testSerial.available())
      ;
    int fanStatus = testSerial.read();
    Serial.println(fanStatus);

    while (!testSerial.available())
      ;
    int fanLevel = testSerial.read();
    Serial.println(fanLevel);

    while (!testSerial.available())
      ;
    int temp = testSerial.read();
    Serial.println(temp);

    while (!testSerial.available())
      ;
    int tempFloat = testSerial.read();
    Serial.println(tempFloat);

    while (!testSerial.available())
      ;
    int humid = testSerial.read();
    Serial.println(humid);

    FirebaseJson json;
    float newTemp = (float)temp + ((float)tempFloat / 100.0);

    json.set("humid", humid);
    json.set("temp", newTemp);

    json.set("Ts/.sv", "timestamp");

    Serial.printf("Push data with timestamp... %s\n", Firebase.pushJSON(fbdo, "/data", json) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set Humidity... %s\n", Firebase.setInt(fbdo, F("/sensors/humid"), humid) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set temp... %s\n", Firebase.setFloat(fbdo, F("/sensors/temp"), newTemp) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set fan... %s\n", Firebase.setFloat(fbdo, F("/sensors/fan"), fanLevel) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set timestamp... %s\n", Firebase.setTimestamp(fbdo, "/sensors/timestamp") ? "ok" : fbdo.errorReason().c_str());
  }
}
