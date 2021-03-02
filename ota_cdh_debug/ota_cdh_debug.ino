#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DieselHeater.h>
#include <WebServer.h>

DieselHeater heater;

const char* ssid = "Glide0183515-2G";
const char* password = "07C8B1112E";

WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /* END OF OTA */

  server.begin();
  server.on("/", handleRoot);

  heater.init();
  heater.setMinPumpHz(1.6);
  heater.setMaxPumpHz(5.5);
  heater.setMinFanSpeed(1680);
  heater.setMaxFanSpeed(3500);
  
}

void handleRoot() {
  // Read posted data and process it
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "state") {
      if (server.arg(i) == "off") {
        heater.turnOff();
      }
      else if (server.arg(i) == "on") {
        heater.turnOn();
      }
    }
  }

  String html = "<html><head><title>Heater</title></head><body><div style=\"width:400px; margin: 0 auto;\"><h1>Basic debug and control</h1><ul>";
  
  html += "<li>Requested state: " + String(heater.getRequestedState()) + "</li>";
  html += "<li>Actual state: " + String(heater.getHtrState()) + "</li>";
  html += "<li>Supply voltage: " + String(heater.getSupplyVoltage()) + "</li>";
  html += "<li>Min Hz: " + String(heater.getMinPumpHz()) + "</li>";
  html += "<li>Max Hz: " + String(heater.getMaxPumpHz()) + "</li>";
  html += "<li>Requested Hz: " + String(heater.getRequestedPumpHz()) + "</li>";
  html += "<li>Current Hz: " + String(heater.getCurrentPumpHz()) + "</li>";
  html += "<li>Glowplug current: " + String(heater.getGlowPlugCurrent()) + "</li>";
  html += "<li>Glowplug voltage: " + String(heater.getGlowPlugVoltage()) + "</li>";
  html += "<li>Min fan speed: " + String(heater.getMinFanSpeed()) + "</li>";
  html += "<li>Max fan speed: " + String(heater.getMaxFanSpeed()) + "</li>";
  html += "<li>Current fan speed: " + String(heater.getFanSpeed()) + "</li>";
  html += "<li>Fan voltage: " + String(heater.getFanVoltage()) + "</li>";
  html += "<li>Body temp: " + String(heater.getHeatExchangerTemp()) + "</li>";
  
  html += "</ul></div><form method=\"POST\">";
  
  html += "<button name=\"state\" value=\"on\">Turn on</html>";
  html += "<button name=\"state\" value=\"off\">Turn off</html>";
  
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
