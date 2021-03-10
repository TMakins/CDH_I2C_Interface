/*
 *  SPIFFS_hz_control.ino
 *  ---------------------
 *  Copyright(C) 2021 Toby Makins
 *  toby@makins.org.uk
 *
 *  Intended platform: ESP32 
 * 
 *  Example sketch demonstrating a SPIFFS stored webpage for controlling a Chinese
 *  Diesel Heater using the CDHInterface library. This sketch implements a webpage 
 *  allowing a user to start and stop a heater, adjust the pump frequency and see
 *  the temperature of the heaters heat exchangers. 
 *
 *  For more info, see the GitHub repository here: https://github.com/TMakins/CDHInterface
 *
 *  This file is part of the CDHInterface library. 
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <DieselHeater.h>

DieselHeater heater;

// The SSID and passkey for the AP
const char* ssid = "Glide0183515-2G";
const char* password = "07C8B1112E";

WebServer server(80);

// IMPORTANT - THESE MAY NOT BE CORRECT FOR YOUR HEATER
// Change these to your current heater settings
const float MIN_HZ = 1.6;
const float MAX_HZ = 5.5;
const uint16_t MIN_FAN_SPEED = 1680; 
const uint16_t MAX_FAN_SPEED = 5000;

float cur_hz = 2.2;

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
      if (ArduinoOTA.getCommand() == U_SPIFFS) {
        SPIFFS.end();
      }
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

  heater.init();
  heater.setMinPumpHz(MIN_HZ);
  heater.setMaxPumpHz(MAX_HZ);
  heater.setMinFanSpeed(MIN_FAN_SPEED);
  heater.setMaxFanSpeed(MAX_FAN_SPEED);

  heater.usePumpHzMode();
  heater.setDesiredPumpHz(cur_hz);

  if (!SPIFFS.begin(true)) {
    return;
  }

  server.begin();

  // Catch all for all
  server.on("/", handleRoot);
  server.onNotFound(handleWebRequests);
}

// Root is the controller, so serve index.html with our placeholders replaced
// and process any data sent to us via POST
void handleRoot() {
  // Read posted data and process it
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "hz") {
      cur_hz = server.arg(i).toFloat();
      heater.setDesiredPumpHz(cur_hz);
    }
    if (server.argName(i) == "state") {
      if (server.arg(i) == "off") {
        heater.turnOff();
      }
      else if (server.arg(i) == "on") {
        heater.turnOn();
      }
    }
  }

  const char *run_state_str;
  if (heater.hasError()) {
    run_state_str = heater.getErrorDesc();
  }
  else {
    run_state_str = heater.getRunStateDesc();
  }

  // Send response with placeholders replaced
  File file = SPIFFS.open("/index.html");
  String html = file.readString();
  html.replace("%%MAX_HZ%%", String(MAX_HZ));
  html.replace("%%MIN_HZ%%", String(MIN_HZ));
  html.replace("%%CUR_HZ%%", String(cur_hz));
  html.replace("%%ON_OFF%%", String(heater.isOn()));
  html.replace("%%RUN_STATE%%", (heater.hasError()) ? heater.getErrorDesc() : heater.getRunStateDesc());
  html.replace("%%BODY_TEMP%%", String(heater.getHeatExchangerTemp()));
  
  html.replace("%%ACT_MIN_HZ%%", String(heater.getMinPumpHz()));
  html.replace("%%ACT_MAX_HZ%%", String(heater.getMaxPumpHz()));
  html.replace("%%ACT_REQ_HZ%%", String(heater.getRequestedPumpHz()));
  html.replace("%%ACT_HZ%%", String(heater.getCurrentPumpHz()));
  html.replace("%%GP_CURRENT%%", String(heater.getGlowPlugCurrent()));
  html.replace("%%GP_VOLTAGE%%", String(heater.getGlowPlugVoltage()));
  html.replace("%%ACT_MAX_RPM%%", String(heater.getMaxFanSpeed()));
  html.replace("%%ACT_MIN_RPM%%", String(heater.getMinFanSpeed()));
  html.replace("%%FAN_SPEED%%", String(heater.getFanSpeed()));
  html.replace("%%FAN_VOLTAGE%%", String(heater.getFanVoltage()));
  html.replace("%%SUP_VOLTAGE%%", String(heater.getSupplyVoltage()));
  file.close();
  server.send(200, "text/html", html);
}


// Act as a web server and serve the required files from SPIFFS
// If you add additional file types to the demo (e.g. images), make sure to 
// add them here too
void handleWebRequests() {
  String dataType = "text/plain";
  String path = server.uri();
  if (path.endsWith("/") || path.endsWith("index.html")) {
    handleRoot();
    return;
  }

  else if (path.endsWith(".css")) {
    dataType = "text/css";
  }
  else if (path.endsWith(".js")) {
    dataType = "application/javascript";
  }
  else if (path.endsWith(".svg")) {
    dataType = "image/svg+xml";
  }
  File file = SPIFFS.open(path, "r");
  server.streamFile(file, dataType);
  file.close();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
