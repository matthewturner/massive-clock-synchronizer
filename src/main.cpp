#include "main.h"

void setup()
{
  Serial.begin(115200);
  delay(10);

  schedule = F("Pending...");

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

  Serial.println();
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

  commandListener.when("update-schedule", (EvtCommandAction)forceUpdateSchedule);

  mgr.addListener(&commandListener);
  mgr.addListener(&handleWifiClientListener);
  mgr.addListener(&updateTimeListener);
  mgr.addListener(&updateScheduleListener);
}

bool forceUpdateSchedule(IEvtListener *, IEvtContext *, long data)
{
  Serial.println("Command: XXX");
  return true;
}

bool updateSchedule()
{
  Serial.println(F("Command: UPDATE"));
  RequestOptions options;
  options.method = F("GET");
  options.headers[F("Connection")] = F("keep-alive");
  options.headers[F("accept")] = F("text/plain");

  Serial.println(F("Sending the request..."));
  Serial.println(F("New schedule:"));

  Response response = fetch(scheduleUrl, options);
  schedule = response.text();
  int startIndex = 0;
  int endIndex = schedule.indexOf('\r');
  Serial.println(schedule.substring(startIndex, endIndex));
  startIndex = endIndex + 1;
  endIndex = schedule.length() - 1;
  Serial.println(schedule.substring(startIndex, endIndex));

  timeClient.forceUpdate();

  return true;
}

bool updateTime()
{
  timeClient.update();
  Serial.print(F("Time: "));
  Serial.println(timeClient.getFormattedTime());
  return true;
}

bool handleWifiClient()
{
  // Check if a client has connected
  WiFiClient client = server.accept();
  if (!client)
  {
    return true;
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
  if (request.indexOf(F("/set=force")) != -1)
  {
    Serial.print(F(">set:"));
    Serial.print(timeClient.getEpochTime());
    Serial.println(F("!"));
    Serial.print(F(">set-schedule:"));
    Serial.print(0);
    Serial.println(F("!"));
  }

  // Return the response
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(); // do not forget this one
  client.println(F("<!DOCTYPE HTML>"));
  client.println(F("<html>"));
  client.println(F("<body>"));

  client.print(F("<p>UTC: "));
  client.print(timeClient.getFormattedTime());
  client.println(F("</p>"));
  client.print(F("<p>Epoch: "));
  client.print(timeClient.getEpochTime());
  client.println(F("</p>"));

  client.print(F("<p>"));
  client.print(F("<a href=\""));
  client.print(scheduleUrl);
  client.print(F("\">Current Schedule</a>"));
  client.print(F(" <a href=\""));
  client.print(scheduleSourceUrl);
  client.print(F("\">Schedule Source</a>"));
  client.println(F("</p>"));

  client.print(F("<p>"));
  client.print(schedule);
  client.println(F("</p>"));

  client.print(F("<p>"));
  client.print(F("To set the clock, click <a href=\"/set=force\">here</a>"));
  client.println(F("</p>"));
  client.println(F("</body>"));
  client.println(F("</html>"));

  delay(1);
  Serial.println(F("Client disconnected"));
  Serial.println();

  return true;
}

void loop()
{
  mgr.loopIteration();
}