#ifndef HTTPed_H
#define HTTPed_H

#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>

struct HTTPResponse {
  String buffer;
  String content;
  int status;
  bool done;
  bool COMPLETED;
};

void connectWifi(char ssid[], char pass[]);
HTTPResponse httpRequest(String server, String path);
void httpProcess(HTTPResponse &r);

#endif