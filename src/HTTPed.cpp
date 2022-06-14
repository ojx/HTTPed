#include "HTTPed.h"

int status = WL_IDLE_STATUS;
WiFiClient client;
//WiFiSSLClient client;

void connectWifi(char ssid[], char pass[]) {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);

    delay(10000); // wait 10 seconds for connection
  }

  Serial.print("Connected. IP Address: ");
  Serial.println(WiFi.localIP());
}

HTTPResponse httpRequest(String server, String path) {
  char svr[server.length() + 1];
  server.toCharArray(svr, server.length() + 1);

  Serial.println("Connecting to server: " + server);

  if (client.connectSSL(svr, 443)) {
    Serial.println("Connected to " + server);
    client.println("GET " + path + " HTTP/1.1");
    client.println("Host: " + server);
    client.println("Referer: " + server);
    client.println("Connection: close");
    client.println();
    Serial.println("Success!");
    // Serial.println(client.connected());
  } else {
    Serial.println("Connection failure: " + server);
  }
  return {"", "", -1, false, false};
}

void httpProcess(HTTPResponse &r) {
  if (!r.done && !client.connected()) {
	
  }

  if (r.done && r.COMPLETED) {
    r.COMPLETED = false;
  }

  if (client.available()) {
    while (client.available()) {
      char c = client.read();
      if (int(c) != 13) {
        r.buffer += c;
      }
      if (r.status != client.status()) {
        Serial.print("New status: ");
        Serial.println(client.status());
        r.status = client.status();
      }

      // Serial.write(c); // for debugging
    }

    if (!r.done && r.status == 4) {
      r.buffer.trim();
      if (r.buffer.indexOf("HTTP/1.1 30") == 0) { //redirect
        int i = r.buffer.indexOf("Location: ");
        String loc = r.buffer.substring(i + 9);
        int ni = loc.indexOf("\n");

        if (ni >= 0) {
          loc = loc.substring(0, ni);
          loc.trim();

          i = loc.indexOf("://");
          loc = loc.substring(i + 3);
          i = loc.indexOf("/");
          String server = loc.substring(0, i);
          String path = loc.substring(i);

          Serial.println("Redirecting to: " + server);
          // Serial.println("Path: " + path);
          client.stop();

          HTTPResponse tempResponse = httpRequest(server, path);
          r.content = tempResponse.content;
          r.buffer = tempResponse.buffer;
          r.status = tempResponse.status;
          r.done = tempResponse.done;
        }
      } else if (r.buffer.indexOf("HTTP/1.1 200") == 0) {
        String c = r.buffer;

        int i = c.indexOf("\n\n");

        if (i < 0) {
          r.content = c;
        } else {
          c = c.substring(i + 2);
          i = c.indexOf("\n");

          if (i < 0) {
            r.content = c;
          } else {
            c = c.substring(i + 1, c.length() - 1);
            c.trim();
            r.content = c;
          }
        }
        r.COMPLETED = true;
        r.done = true;
        //  Serial.println(r.content);
        client.stop();
      }
    }
  }
}
