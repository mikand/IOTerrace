#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>


#define SENSOR_PIN           A0
#define DRY_SENSOR_READ      854
#define WATER_SENSOR_READ    451
#define SENSOR_READ_INTERVAL 2000


WiFiClient wifiClient;

struct {
  char username[40] = "";
  char password[40] = "";
} credentials;

int sensorRead = 0;
int moisture = 0;


void setup(void) {
  WiFiManager wifiManager;

  Serial.begin(9600);
  EEPROM.begin(512);

  EEPROM.get(0, credentials);

  WiFiManagerParameter username("username", "Wifi username", credentials.username, 40);
  wifiManager.addParameter(&username);
  WiFiManagerParameter password("password", "Wifi password", credentials.password, 40);
  wifiManager.addParameter(&password);

  wifiManager.autoConnect("GardenManager");

  Serial.println(F("WiFi connected! IP address: "));
  Serial.println(WiFi.localIP());

  strncpy(credentials.username, username.getValue(), 40);
  strncpy(credentials.password, password.getValue(), 40);

  EEPROM.put(0, credentials);
  EEPROM.commit();
}


void loop(void) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("*** Disconnected from AP so rebooting ***"));
    ESP.reset();
  }

  sensorRead = analogRead(SENSOR_PIN);
  moisture = map(sensorRead, DRY_SENSOR_READ, WATER_SENSOR_READ, 0, 100);
  Serial.println(moisture);

  delay(SENSOR_READ_INTERVAL);
}
