#include "main.h"

void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  initializeSchedule();

  // Connect to WiFi network
  DEBUG_PLN();
  DEBUG_PLN();
  DEBUG_P(F("Connecting to "));
  DEBUG_PLN(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    DEBUG_P(F("."));
  }

  DEBUG_PLN();
  DEBUG_PLN(F("WiFi connected!"));

  // Start the server
  server.begin();
  DEBUG_PLN(F("Server started"));

  // Print the IP address
  DEBUG_P(F("Use this URL to connect: "));
  DEBUG_P(F("http://"));
  DEBUG_P(WiFi.localIP());
  DEBUG_PLN(F("/"));

  timeClient.setUpdateInterval(TIME_UPDATE_INTERVAL);
  timeClient.begin();

  commandListener.when("request-sync", (EvtCommandAction)sync);

  mgr.addListener(&commandListener);
  mgr.addListener(&handleWifiClientListener);
  mgr.addListener(&updateTimeListener);

  updateTime();
}

bool sync(IEvtListener *, IEvtContext *, long data)
{
  updateSchedule();
  updateTime();
  pushUpdate();
  return true;
}

bool updateSchedule()
{
  digitalWrite(LED, LOW);
  DEBUG_PLN(F("Updating schedule..."));

  RequestOptions options;
  options.method = F("GET");
  options.headers[F("Connection")] = F("keep-alive");
  options.headers[F("accept")] = F("text/plain");

  DEBUG_PLN(F("Sending the request..."));
  DEBUG_PLN(F("New schedule:"));

  Response response = fetch(scheduleUrl, options);

  if (response.status != 200)
  {
    return false;
  }

  initializeSchedule();

  String scheduleDef = response.text();
  int startIndex = 0;
  int endIndex = scheduleDef.indexOf('\r', startIndex);

  for (byte i = 0; i < MAX_SCHEDULES; i++)
  {
    if (endIndex == -1)
    {
      break;
    }
    schedule[i] = atol(scheduleDef.substring(startIndex, endIndex).c_str());
    DEBUG_PLN(schedule[i]);
    startIndex = endIndex + 1;
    endIndex = scheduleDef.indexOf('\r', startIndex);
  }

  digitalWrite(LED, HIGH);

  return true;
}

bool updateTime()
{
  timeClient.update();
  // DEBUG_P(F("Time: "));
  // DEBUG_PLN(timeClient.getFormattedTime());
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
  DEBUG_PLN(F("New client!"));
  while (!client.available())
  {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  DEBUG_PLN(request);
  client.flush();

  // Match the request
  if (request.indexOf(F("/force-sync")) != -1)
  {
    updateSchedule();
    updateTime();
    pushUpdate();
  }

  // Return the response
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(); // do not forget this one
  client.println(F("<!DOCTYPE HTML>"));
  client.println(F("<html>"));
  client.println(F("<body>"));

  client.println(F("<h2>Time</h2>"));

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
    client.println(F("<p>Pending retrieval... Try again shortly.</p>"));
  }

  client.print(F("<p>"));
  client.print(F("To update the clock now, click <a href=\"/force-sync\">here</a>"));
  client.println(F("</p>"));
  client.println(F("</body>"));
  client.println(F("</html>"));

  delay(1);
  DEBUG_PLN(F("Client disconnected"));
  DEBUG_PLN();

  return true;
}

void pushUpdate()
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