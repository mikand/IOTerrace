#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <DHT.h>
#include <WiFiClientSecureBearSSL.h>

#include <SPIFFSCertStoreFile.h>

#define SENSOR_PIN           A0
#define DRY_SENSOR_READ      854
#define WATER_SENSOR_READ    451
#define SENSOR_READ_INTERVAL 5000

#define DHT_PIN D1

const int SELECTION_PINS[] = {D6, D7, D8};
#define NUM_SELECTION_PINS 3
#define NUM_SENSORS 8
#define MULTIPLEXER_DELAY 1000

const String HOSTNAME = "https://ioterrace.micheli.website";

WiFiManager wifiManager;
WiFiClient wifiClient;
DHT dht(DHT_PIN, DHT22);

struct {
  char username[40] = "";
  char password[40] = "";
} credentials;

int sensorRead = 0;
int moisture = 0;

float temperature = 0;
float humidity = 0;

SPIFFSCertStoreFile certs_idx("/certs.idx");
SPIFFSCertStoreFile certs_ar("/certs.ar");

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void sendMeasure(int sensorId, float data)
{
  HTTPClient http;
  BearSSL::WiFiClientSecure client;
  BearSSL::CertStore certStore;
  int numCerts = certStore.initCertStore(&certs_idx, &certs_ar);
  client.setCertStore(&certStore);
  Serial.println(numCerts);

  http.begin(client, HOSTNAME + "/add_reading/" + sensorId + "/" + data);
  int httpCode = http.GET();
  Serial.println("Response code:");
  Serial.println(httpCode);
  http.end();
}

void setup(void) {
  pinMode(DHT_PIN, INPUT);

  for (unsigned i=0; i<NUM_SELECTION_PINS; i++) {
    pinMode(SELECTION_PINS[i], OUTPUT);
  }

  dht.begin();


  Serial.begin(9600);
  EEPROM.begin(512);

  Serial.println("IOTerrace Starting!");

  EEPROM.get(0, credentials);

  WiFiManagerParameter username("username", "Wifi username",
                                credentials.username, 40);
  wifiManager.addParameter(&username);
  WiFiManagerParameter password("password", "Wifi password",
                                credentials.password, 40);
  wifiManager.addParameter(&password);

  wifiManager.autoConnect("IOTerrace");

  Serial.println(F("WiFi connected! IP address: "));
  Serial.println(WiFi.localIP());

  strncpy(credentials.username, username.getValue(), 40);
  strncpy(credentials.password, password.getValue(), 40);

  EEPROM.put(0, credentials);
  EEPROM.commit();

  setClock();
  SPIFFS.begin();
}


void loop(void) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("*** Disconnected from AP so rebooting ***"));
    ESP.reset();
  }

  for (unsigned i=0; i<NUM_SENSORS; i++) {
    for (unsigned bit=0; bit<NUM_SELECTION_PINS; bit++) {
      digitalWrite(SELECTION_PINS[bit], bitRead(i, bit) ? HIGH : LOW);
    }
    delay(MULTIPLEXER_DELAY);

    sensorRead = analogRead(SENSOR_PIN);
    moisture = map(sensorRead, DRY_SENSOR_READ, WATER_SENSOR_READ, 0, 100);

    sendMeasure(i, moisture);
  }

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  Serial.println(temperature);
  Serial.println(humidity);
  Serial.println(F("----"));

  sendMeasure(9, temperature);
  sendMeasure(9, humidity);

  delay(SENSOR_READ_INTERVAL);
}
