#if defined(ESP32)
#include <WiFi.h>
// #include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP8266.h>
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#endif

EspSoftwareSerial::UART testSerial;
// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Surapee"
#define WIFI_PASSWORD "21102545"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyDVomwtqAWlUln7ft-qbkgpWVecurVZrQE"

/* 3. Define the RTDB URL */
#define DATABASE_URL "nodemcu-f263e-default-rtdb.firebaseio.com"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "admin@admin.com"
#define USER_PASSWORD "thisispassword"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

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

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  // otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////

  Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
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

  /** Timeout options.

  //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;

  //Socket connection and SSL handshake timeout in ms (1 sec - 1 min).
  config.timeout.socketConnection = 10 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  Note:
  The function that starting the new TCP session i.e. first time server connection or previous session was closed, the function won't exit until the
  time of config.timeout.socketConnection.

  You can also set the TCP data sending retry with
  config.tcp_data_sending_retry = 1;

  */
  timeClient.begin();
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

    while (!testSerial.available());
    int fanStatus = testSerial.read();
    Serial.println(fanStatus);

    while (!testSerial.available());
    int fanLevel = testSerial.read();
    Serial.println(fanLevel);

    while (!testSerial.available());
    int temp = testSerial.read();
    Serial.println(temp);

    while (!testSerial.available());
    int tempFloat = testSerial.read();
    Serial.println(tempFloat);

    while (!testSerial.available());
    int humid = testSerial.read();
    Serial.println(humid);


    FirebaseJson json;
    float newTemp = (float)temp + ((float)tempFloat/100.0);
    int maxHumid = 100;
    int minHumid = 10;
    int maxTemp = 50;
    int minTemp = 20;
    if (humid < maxHumid && humid > minHumid && newTemp < maxTemp && newTemp > minTemp && fanLevel < 4 && fanLevel >= 0) {
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
}

/// PLEASE AVOID THIS ////

// Please avoid the following inappropriate and inefficient use cases
/**
 *
 * 1. Call get repeatedly inside the loop without the appropriate timing for execution provided e.g. millis() or conditional checking,
 * where delay should be avoided.
 *
 * Everytime get was called, the request header need to be sent to server which its size depends on the authentication method used,
 * and costs your data usage.
 *
 * Please use stream function instead for this use case.
 *
 * 2. Using the single FirebaseData object to call different type functions as above example without the appropriate
 * timing for execution provided in the loop i.e., repeatedly switching call between get and set functions.
 *
 * In addition to costs the data usage, the delay will be involved as the session needs to be closed and opened too often
 * due to the HTTP method (GET, PUT, POST, PATCH and DELETE) was changed in the incoming request.
 *
 *
 * Please reduce the use of swithing calls by store the multiple values to the JSON object and store it once on the database.
 *
 * Or calling continuously "set" or "setAsync" functions without "get" called in between, and calling get continuously without set
 * called in between.
 *
 * If you needed to call arbitrary "get" and "set" based on condition or event, use another FirebaseData object to avoid the session
 * closing and reopening.
 *
 * 3. Use of delay or hidden delay or blocking operation to wait for hardware ready in the third party sensor libraries, together with stream functions e.g. Firebase.RTDB.readStream and fbdo.streamAvailable in the loop.
 *
 * Please use non-blocking mode of sensor libraries (if available) or use millis instead of delay in your code.
 *
 * 4. Blocking the token generation process.
 *
 * Let the authentication token generation to run without blocking, the following code MUST BE AVOIDED.
 *
 * while (!Firebase.ready()) <---- Don't do this in while loop
 * {
 *     delay(1000);
 * }
 *
 */
