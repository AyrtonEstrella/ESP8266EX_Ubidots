#include <ESP8266EX.h>
#include <SoftwareSerial.h>

#define ESP_RX   3
#define ESP_TX   2
#define ESP_RST  4
SoftwareSerial Serial1(ESP_RX, ESP_TX);
ESP8266EX   wifi(&Serial1, &Serial);

#define WIFI_SSID "INFINITUMzgh4"
#define WIFI_PASS "a9a9c4e621"

#define UbidotsHost "things.ubidots.com"
#define UbidotsPort "80"
#define UbidotsUrl  "/api/v1.6/devices/"
#define UbidotsDevice "device"
#define UbidotsAuth "A1E-ebb4e92ca426b6d4225c1d66a5bf08616fea"
#define UbidotsToken "A1E-v5RuBcWaP6dCNDv16IeSc7XEvQ6KMt"
#define UbidotsVar1 "lugar"
#define UbidotsVar2 "peso"
#define UbidotsVar3 "volt"

void setup() {
  Serial1.begin(9600);                  // Soft serial connection to ESP8266
  Serial.begin(9600);

  wifi.setBootMarker("Ai-Thinker Technology Co. Ltd.");
  wifi.setModuleEcho(false);
  wifi.softReset();
  wifi.setModuleEcho(false);
  wifi.getVersion();
  wifi.setWifiMode(wifi.STATION_MODE);
  wifi.setConnectionMode(wifi.SINGLE_CONNECTION);
  if (wifi.connectToAP(WIFI_SSID, WIFI_PASS) == false)
    return;

  wifi.setUbidotsToken(UbidotsToken);
  wifi.setUbidotsURL(UbidotsUrl);
  wifi.setVariablesNames(UbidotsVar1, UbidotsVar2, UbidotsVar3);

  if (wifi.connectTCP(UbidotsHost, UbidotsPort) == false)
    return;

  wifi.ubidotsPostRequest(UbidotsDevice, "1", "900", "11");
  wifi.closeTCP();
  wifi.closeAP();
}

void loop() {

}
