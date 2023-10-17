#include "main.h"

void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected!"));

  // Start the server
  server.begin();
  Serial.println(F("Server started"));

  // Print the IP address
  Serial.print(F("Use this URL to connect: "));
  Serial.print(F("http://"));
  Serial.print(WiFi.localIP());
  Serial.println(F("/"));

  timeClient.setUpdateInterval(60 * 1000 * 5);
  timeClient.begin();
}

void loop()
{
  // Check if a client has connected
  WiFiClient client = server.accept();
  if (!client)
  {
    return;
  }

  // Wait until the client sends some data
  Serial.println(F("New client!"));
  while (!client.available())
  {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request

  int value = LOW;
  if (request.indexOf(F("/LED=ON")) != -1)
  {
    digitalWrite(ledPin, HIGH);
    value = HIGH;
  }
  if (request.indexOf(F("/LED=OFF")) != -1)
  {
    digitalWrite(ledPin, LOW);
    value = LOW;
  }

  // Set ledPin according to the request
  // digitalWrite(ledPin, value);

  // Return the response
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(F("")); // do not forget this one
  client.println(F("<!DOCTYPE HTML>"));
  client.println(F("<html>"));
  client.println(F("<body>"));

  timeClient.update();

  client.print(F("<p>"));
  client.print(timeClient.getFormattedTime());
  client.println(F("</p>"));
  client.print(F("<p>"));
  client.print(timeClient.getEpochTime());
  client.println(F("</p>"));

  client.print(F("<p>"));
  client.print(F("Led pin is now: "));

  if (value == HIGH)
  {
    client.print(F("On"));
  }
  else
  {
    client.print(F("Off"));
  }
  client.println(F("</p>"));
  client.print(F("<p>"));
  client.print(F("Click <a href=\"/LED=ON\">here</a> to turn the LED on pin 2 ON"));
  client.println(F("</p>"));
  client.print(F("<p>"));
  client.print(F("Click <a href=\"/LED=OFF\">here</a> to turn the LED on pin 2 OFF"));
  client.println(F("</p>"));
  client.println(F("</body>"));
  client.println(F("</html>"));

  delay(1);
  Serial.println(F("Client disconnected"));
  Serial.println(F(""));
}
