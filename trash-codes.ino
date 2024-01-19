
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
WiFiClient client;
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <DHT.h>
#include <SimpleTimer.h>

LiquidCrystal_I2C mylcd(0x27,16,2);
volatile int Temp;
volatile int Humid;
volatile int Rain;
volatile int UV;
String AABB;
String AACC;
String TouchGrass;
Adafruit_MQTT_Client mqtt(&client, "your mqtt site", port, "username", "PASSWORD");
void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

int8_t timeZone = 8;
const PROGMEM char *ntpServer = "ntp1.aliyun.com";
DHT dht5(5, 11);
Adafruit_MQTT_Publish MQTT_Topic_HotBaka = Adafruit_MQTT_Publish(&mqtt, "temperature feeds name");
Adafruit_MQTT_Publish MQTT_Topic_WetBaka = Adafruit_MQTT_Publish(&mqtt, "humidity feeds name");
Adafruit_MQTT_Publish MQTT_Topic_UVBaka = Adafruit_MQTT_Publish(&mqtt, "UV feeds name");
SimpleTimer timer;

void Simple_timer_1() {
  MQTT_Topic_HotBaka.publish(Temp);
   MQTT_Topic_WetBaka.publish(Humid);
   MQTT_Topic_UVBaka.publish(UV);
}

void setup(){
  mylcd.init();
  mylcd.backlight();
  Temp = 0;
  Humid = 0;
  Rain = 0;
  UV = 0;
  AABB = "";
  AACC = "";
  TouchGrass = "";
  WiFi.begin("wifi name", "wifi password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Local IP:");
  Serial.print(WiFi.localIP());

  NTP.setInterval (600);
  NTP.setNTPTimeout (1500);
  NTP.begin (ntpServer, timeZone, false);
  Serial.begin(9600);
  MQTT_connect();
  Serial.println("");
   dht5.begin();
  timer.setInterval(60000L, Simple_timer_1);

  pinMode(25, INPUT);
  pinMode(33, INPUT);
}

void loop(){
  Temp = dht5.readTemperature();
  Humid = dht5.readHumidity();
  Rain = analogRead(34);
  UV = analogRead(35);

  timer.run();

  // Touch some grass LOL
  if (Rain <= 3000 && UV > 80) {
    TouchGrass = String("Sure,Go touch grass         ");

  } else {
    TouchGrass = String("umm,still touch some grass");

  }
  if (Rain <= 3000) {
    AABB = "Yes         ";

  } else {
    AABB = "No         ";

  }
  if (UV > 80) {
    AACC = "High";

  } else if (UV > 50) {
    AACC = "Mid";
  } else {
    AACC = "Low";

  }
  // This is impossible
  if ((digitalRead(25) && digitalRead(33)) == 1) {
    mylcd.setCursor(0, 0);
    mylcd.print("TouchGrass?         ");
    mylcd.setCursor(0, 1);
    mylcd.print(TouchGrass);

  } else if (digitalRead(33) == 1) {
    mylcd.setCursor(0, 0);
    mylcd.print(String("Date:") + String(NTP.getDateYear()) + String("/") + String(NTP.getDateMonth()) + String("/") + String(NTP.getDateDay()) + String("         "));
    mylcd.setCursor(0, 1);
    mylcd.print(String("Week:") + String(NTP.getDateWeekday()) + String("          "));
  } else if (digitalRead(25) == 1) {
    mylcd.setCursor(0, 0);
    mylcd.print(String("Raining?") + String(AABB));
    mylcd.setCursor(0, 1);
    mylcd.print(String("UV level:") + String(AACC) + String('(') + String(UV) + String(')') + String("  "));
  } else {
    mylcd.setCursor(0, 0);
    mylcd.print(String("Time:") + String(NTP.getTimeHour24()) + String(":") + String(NTP.getTimeMinute()) + String("      "));
    mylcd.setCursor(0, 1);
    mylcd.print(String("Temp:") + String(Temp) + String(" Humid:") + String(Humid));

  }

}
