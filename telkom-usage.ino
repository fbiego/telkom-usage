/*

  MIT License

  Copyright (c) 2021 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <OLED_I2C.h>

WiFiMulti wifiMulti;
OLED  myOLED(18, 19); //(SDA, SCL)

// connect your ESP32 to the internet
char* WIFI_SSID = "your_ssid_here";
char* WIFI_PASS = "your_password_here";

String ACC_ID = "your_account_id_here"; // see README.md

extern uint8_t SmallFont[], MediumNumbers[];

unsigned int bun, crd, zad;

void setup() {
  Serial.begin(115200);

  if (!myOLED.begin(SSD1306_128X32)) {
    while (1);  // In case the library failed to allocate enough RAM for the display buffer...
  }
  myOLED.setFont(SmallFont);
  Serial.println("Starting...");

  myOLED.clrScr();
  myOLED.print("Connecting...", LEFT, 12);
  myOLED.update();

  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);

  // wait for wifi to connect
  while ((wifiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }

  Serial.println("Connected");

}

void loop() {

  getInfo();

  myOLED.clrScr();
  myOLED.setFont(SmallFont);
  myOLED.print(String(crd * 0.01) + " Ksh", CENTER, 0);
  myOLED.print("MB", 100, 24);
  myOLED.setFont(MediumNumbers);
  myOLED.print(String((bun / 1024) + 1), CENTER, 15);
  myOLED.update();

  delay(5000);

}

void getInfo() {
  HTTPClient http;
  http.begin("http://myaccount.telkom.co.ke//callservice.do");
  http.addHeader("Referer", "http://myaccount.telkom.co.ke/3G/index.jsp");
  http.addHeader("Origin", "http://myaccount.telkom.co.ke");
  http.addHeader("Host", "myaccount.telkom.co.ke");
  int httpCode = http.POST("<?xml version=\"1.0\" encoding=\"UTF-8\"?><zsmart><ServiceName>QryBalListFilterAllExpire</ServiceName><Data><sACCT_ID>" + ACC_ID + "</sACCT_ID></Data></zsmart>");

  String payload = http.getString();

  // the payload may be different for each user
  // this will affect how the parseBal() function works

  //Serial.println(payload);

  if (httpCode == HTTP_CODE_OK) {
    parseBal(payload);
  } else {
    Serial.println("Error: " + httpCode);
  }
  http.end();

}

void parseBal(String source) {
  String credit = source.substring(source.indexOf("<ACCT_RES_NAME>Main Account</ACCT_RES_NAME>"), source.indexOf("<ACCT_RES_NAME>Ziada Points</ACCT_RES_NAME>"));
  String ziada = source.substring(source.indexOf("<ACCT_RES_NAME>Ziada Points</ACCT_RES_NAME>"), source.indexOf("<ACCT_RES_NAME>Monthly Friday Data</ACCT_RES_NAME>"));
  String bundles = source.substring(source.indexOf("<ACCT_RES_NAME>Monthly Friday Data</ACCT_RES_NAME>"), source.indexOf("<ACCT_RES_NAME>Madaraka FREE Data</ACCT_RES_NAME>"));

  crd = credit.substring(credit.indexOf("<REAL_BAL>") + 11, credit.indexOf("</REAL_BAL>")).toInt();
  zad = ziada.substring(ziada.indexOf("<REAL_BAL>") + 11, ziada.indexOf("</REAL_BAL>")).toInt();
  bun = bundles.substring(bundles.indexOf("<REAL_BAL>") + 11, bundles.indexOf("</REAL_BAL>")).toInt();


  Serial.println("------CREDIT-----");
  Serial.println(crd * 0.01);
  Serial.println("------ZIADA-----");
  Serial.println(zad);
  Serial.println("------BUNDLES-----");
  Serial.println((bun / 1024) + 1);

}
