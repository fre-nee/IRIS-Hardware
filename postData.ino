struct {
  char ssid[32] = "";
  char password[64] = "";
  char node_wifi[10] = "true";
} data;

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#define EEPROM_CONFIG_START 0

ESP8266WebServer server(80);

const int led = 13;
void resetMCU(){
  Serial.println("Resetting ESP");
  ESP.restart();
}

void handleConfig() {
  String json_data = server.arg("plain");
  StaticJsonBuffer<200> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(json_data);
  String wifiSSID = jObject["wifiSSID"];
  String wifiKey = jObject["wifiKey"];
  String message = wifiSSID + "    ----------------    " + wifiKey;

  char ssid_Buf[32];
  char password_Buf[62];
  wifiSSID.toCharArray(ssid_Buf, 32);
  wifiKey.toCharArray(password_Buf, 32);

  strcpy(data.ssid , ssid_Buf);
  strcpy(data.password, password_Buf);
  strcpy(data.node_wifi, "false");

  EEPROM.put(addr, data);
  EEPROM.commit();
  EEPROM.get(addr, data);

  Serial.println("New values are: " + String(data.ssid) + "," + String(data.password));
  server.send(200, "text/plain", "EEPROM set to: " + String(data.ssid) + "," + String(data.password));
  resetMCU();
}

void handleReset() {

  strcpy(data.ssid , "");
  strcpy(data.password, "");
  strcpy(data.node_wifi, "true");

  EEPROM.put(addr, data);
  EEPROM.commit();

  Serial.println("Data reset");
  server.send(200, "text/plain", "Reset Device");
  resetMCU();
}

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void setPins() {
  String data = server.arg("plain");
  StaticJsonBuffer<200> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);
  String pinNumber = jObject["pin"];
  String pinStatus = jObject["status"];
  String message = pinNumber + "  ---------------------------  " + pinStatus;
  switch (pinNumber.toInt()) {
    case 0 :
      Serial.println("D0");
      if (pinStatus == "0") {
        digitalWrite(D0, LOW);
      }
      if (pinStatus == "1") {
        digitalWrite(D0, HIGH);
      }
      break;
    case 1 :
      Serial.println("D1");
      if (pinStatus == "0") {
        digitalWrite(D1, LOW);
      }
      if (pinStatus == "1") {
        digitalWrite(D1, HIGH);
      }
      break;
    case 2 :
      Serial.println("D2");
      if (pinStatus == "0") {
        digitalWrite(D2, LOW);
      }
      if (pinStatus == "1") {
        digitalWrite(D2, HIGH);
      }
      break;
    case 3 :
      Serial.println("D3");
      if (pinStatus == "0") {
        digitalWrite(D3, LOW);
      }
      if (pinStatus == "1") {
        digitalWrite(D3, HIGH);
      }
      break;
    default:
      digitalWrite(D0, HIGH);
      digitalWrite(D1, HIGH);
      digitalWrite(D2, HIGH);
      digitalWrite(D3, HIGH);
  }

  server.send(200, "text/plain", message);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);

  EEPROM.begin(512);
  EEPROM.get(addr, data);
  String match = "true";
  if (match.equals(String(data.node_wifi))) {
    WiFi.mode(WIFI_AP); 
    WiFi.softAP("IOT-HOME");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }

  else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(data.ssid, data.password);
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(data.ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);

  digitalWrite(D0, HIGH);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/setpins", setPins);
  server.on("/config", handleConfig);
  server.on("/reset", handleReset);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
