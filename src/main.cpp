#include "main.h"

void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  initializeSchedule();

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

  commandListener.when("update", (EvtCommandAction)forceUpdateSchedule);

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
  digitalWrite(LED, LOW);
  Serial.println(F("Command: UPDATE"));
  RequestOptions options;
  options.method = F("GET");
  options.headers[F("Connection")] = F("keep-alive");
  options.headers[F("accept")] = F("text/plain");

  Serial.println(F("Sending the request..."));
  Serial.println(F("New schedule:"));

  Response response = fetch(scheduleUrl, options);
  String scheduleDef = response.text();
  int startIndex = 0;
  int endIndex = scheduleDef.indexOf('\r');
  schedule[0] = atol(scheduleDef.substring(startIndex, endIndex).c_str());
  Serial.println(schedule[0]);
  startIndex = endIndex + 1;
  endIndex = scheduleDef.length() - 1;
  schedule[1] = atol(scheduleDef.substring(startIndex, endIndex).c_str());
  Serial.println(schedule[1]);
  timeClient.forceUpdate();

  digitalWrite(LED, HIGH);

  return true;
}

bool updateTime()
{
  timeClient.update();
  // Serial.print(F("Time: "));
  // Serial.println(timeClient.getFormattedTime());
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
    forceUpdate();
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

  client.println(F("<h2>Schedules</h2>"));

  client.print(F("<p>"));
  client.print(F("<a href=\""));
  client.print(scheduleUrl);
  client.print(F("\">Current</a>"));
  client.print(F(" <a href=\""));
  client.print(scheduleSourceUrl);
  client.print(F("\">Source</a>"));
  client.println(F("</p>"));

  bool scheduleRetrieved = false;
  for (byte i = 0; i < MAX_SCHEDULES; i++)
  {
    if (schedule[i] > 0)
    {
      scheduleRetrieved = true;
      client.print(F("<p>"));
      client.print(schedule[i]);
      client.println(F("</p>"));
    }
  }
  if (!scheduleRetrieved)
  {
    client.print(F("<p>Pending retrieval... Try again shortly.</p>"));
  }

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

void forceUpdate()
{
  Serial.print(F(">set:"));
  Serial.print(timeClient.getEpochTime());
  Serial.println(F("!"));
  for (byte i = 0; i < MAX_SCHEDULES; i++)
  {
    if (schedule[i] > 0)
    {
      Serial.print(F(">set-schedule:"));
      Serial.print(schedule[i]);
      Serial.println(F("!"));
    }
  }
}

void initializeSchedule()
{
  for (byte i = 0; i < MAX_SCHEDULES; i++)
  {
    schedule[i] = 0;
  }
}

void loop()
{
  mgr.loopIteration();
}