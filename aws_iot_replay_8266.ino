// Conditional compilation to include platform-specific libraries
#include "secrets.h"
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WiFiMulti.h>
  #include <WiFiClientSecureBearSSL.h> // Use the correct include for ESP8266
  using namespace BearSSL;  // Use BearSSL namespace
  std::unique_ptr<BearSSL::WiFiClientSecure> net(new BearSSL::WiFiClientSecure); // Correctly declare the net object
  std::unique_ptr<BearSSL::X509List> cert(new BearSSL::X509List(cacert));
  std::unique_ptr<BearSSL::X509List> client_crt(new BearSSL::X509List(client_cert));
  std::unique_ptr<BearSSL::PrivateKey> key(new BearSSL::PrivateKey(privkey));
  BearSSL::PrivateKey key(privkey);
  WiFiClientSecure net;
#elif defined(ESP32)
    #include <WiFi.h>
  #include <WiFiClientSecure.h> // Make sure this is included for ESP32
  std::unique_ptr<WiFiClientSecure> net(new WiFiClientSecure); // Correctly declare the net object for ESP32
#endif

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define TIME_ZONE -7 //MST
#define RELAY_PIN 1

#define AWS_IOT_PUBLISH_TOPIC   "esp8266/pub" // Publish state here
#define AWS_IOT_SUBSCRIBE_TOPIC "esp8266/sub" // Subscibe to commands here

bool relayState = false;
unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 5000;
 
PubSubClient client(*net.get()); 

time_t now;
time_t nowish = 1510592825;
 
void NTPConnect(void) {
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}
 
void messageReceived(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Turn the relay on or off depending on the payload
  if (strncmp((char *)payload, "on", length) == 0) {
    relayState = true;
    digitalWrite(RELAY_PIN, HIGH);
  } else if (strncmp((char *)payload, "off", length) == 0) {
    relayState = false; 
    digitalWrite(RELAY_PIN, LOW);
  } else if (strncmp((char *)payload, "toggle", length) == 0) {
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW );
  }

  publishMessage();

}


void connectAWS() {
 
  NTPConnect();
 
  #if defined(ESP8266)
    net->setTrustAnchors(&cert);
    net->setClientRSACert(&client_crt, &key);
  #elif defined(ESP32)
    net->setCACert(cacert);
    net->setCertificate(client_cert);
    net->setPrivateKey(privkey);
  #endif

  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME)){
    Serial.print(".");
    delay(500);
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}
 
 
void publishMessage() {
  Serial.println("Publish Message");
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["state"] = relayState;
  
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
 
void setup() {
  Serial.begin(115200);
  
  // Setup Relay Pin
  pinMode(RELAY_PIN, OUTPUT);

  // Initialize WiFiManager
  WiFiManager wifiManager;
  
  // Fetches SSID and pass from eeprom and tries to connect
  // If it does not connect it starts an access point with the specified name
  // Here "AutoConnectAP" is the name of the AP that will be created if needed
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("Relay ESP MQTT");
  
  // If we get here, we are connected to the WiFi
  Serial.println("Connected to WiFi");

  connectAWS();
}
 
 
void loop(){
 
  now = time(nullptr);
 
  if (!client.connected()) {
    connectAWS();
  }
  else {
    client.loop();
  }

}