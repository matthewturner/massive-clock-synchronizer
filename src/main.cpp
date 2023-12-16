#include "main.h"

void setup()
{
  Serial.begin(9600);
  delay(10);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  initializeSchedule();

  // Connect to WiFi network
  DEBUG_PLN();
  DEBUG_PLN();
  DEBUG_P(F("Connecting to "));
  DEBUG_PLN(ssid);

  WiFi.hostname(F("clock"));
  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (counter > 20)
    {
      break;
    }
    digitalWrite(LED, !digitalRead(LED));
    delay(500);
    Serial.println(F("."));
    counter++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    DEBUG_PLN();
    Serial.println(F("WiFi connected!"));

    // Start the server
    server.begin();
    DEBUG_PLN(F("Server started"));

    // Print the IP address
    DEBUG_P(F("Use this URL to connect: "));
    DEBUG_P(F("http://"));
    DEBUG_P(WiFi.localIP());
    DEBUG_PLN(F("/"));
  }
  else
  {
    Serial.println(F("WiFi failed to connect"));
  }

  timeClient.setUpdateInterval(TIME_UPDATE_INTERVAL);
  timeClient.begin();

  commandListener.when("request-sync", (EvtCommandAction)sync);
  commandListener.when("ping", (EvtCommandAction)pong);

  mgr.addListener(&commandListener);
  mgr.addListener(&handleWifiClientListener);
  mgr.addListener(&pingListener);
  mgr.addListener(&updateTimeListener);
  mgr.addListener(&updateScheduleListener);

  updateTime();
  updateSchedule();

  digitalWrite(LED, LOW);
}

bool sync()
{
  updateTime();
  pushUpdate();
  lastSync = millis();
  return true;
}

bool show()
{
  Serial.println(F(">show!"));
  return true;
}

bool ping()
{
  Serial.println(F(">ping!"));
  return true;
}

bool pong()
{
  Serial.println(F(">pong!"));
  return true;
}

bool updateSchedule()
{
  digitalWrite(LED, HIGH);
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

  digitalWrite(LED, LOW);
  lastScheduleUpdate = millis();

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
  ushort counter = 0;
  while (!client.available())
  {
    delay(1);
    counter++;
    if (counter > 500)
    {
      break;
    }
  }

  if (!client.available())
  {
    DEBUG_P(F("Client did not complete request"));
    client.stop();
    return true;
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  DEBUG_PLN(request);
  client.flush();

  // Match the request
  if (request.indexOf(F("/v1/status")) != -1)
  {
    outputStatusAsJson(&client);
  }
  else if (request.indexOf(F("/force-sync")) != -1)
  {
    sync();
    outputStatusAsHtml(&client);
  }
  else if (request.indexOf(F("/update-schedule")) != -1)
  {
    updateSchedule();
    outputStatusAsHtml(&client);
  }
  else if (request.indexOf(F("/show")) != -1)
  {
    show();
    outputStatusAsHtml(&client);
  }
  else
  {
    outputStatusAsHtml(&client);
  }

  delay(1);

  client.stop();

  DEBUG_PLN(F("Client disconnected"));
  DEBUG_PLN();

  return true;
}

void formatEpochAsUtc(char buff[32], unsigned long epochTime)
{
  sprintf(buff, "%02d-%02d-%02dT%02d:%02d:%02dZ", year(epochTime), month(epochTime), day(epochTime), hour(epochTime), minute(epochTime), second(epochTime));
}

void outputStatusAsJson(WiFiClient *pClient)
{
  WiFiClient client = *pClient;

  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));

  unsigned long now = millis();

  unsigned long epochTime = timeClient.getEpochTime();
  char buff[32];
  formatEpochAsUtc(buff, epochTime);

  String body = F("{\n");
  body += F("\"utc\": \"");
  body += buff;
  body += "\",\n";
  body += F("\"epoch\": ");
  body += String(epochTime);
  body += ",\n";

  bool scheduleRetrieved = false;
  body += F("\"schedules\": [");
  for (byte i = 0; i < MAX_SCHEDULES; i++)
  {
    if (schedule[i] > 0)
    {
      scheduleRetrieved = true;
      body += F("\n");
      body += String(schedule[i]);
      body += F(",");
    }
  }
  if (body.endsWith(","))
  {
    int lastIndex = body.length() - 1;
    body.remove(lastIndex);
    body += F("\n");
  }
  body += F("],\n");

  body += F("\"scheduleUpdatedAt\": ");
  if (scheduleRetrieved)
  {
    unsigned long lastScheduleUpdateAgoSeconds = (now - lastScheduleUpdate) / 1000;
    unsigned long scheduleUpdatedAt = epochTime - lastScheduleUpdateAgoSeconds;
    formatEpochAsUtc(buff, scheduleUpdatedAt);
    body += F("\"");
    body += String(buff);
    body += F("\"");
  }
  else
  {
    body += F("null");
  }
  body += F(",\n");

  body += F("\"clockSyncedAt\": ");
  if (lastSync == 0)
  {
    body += F("null");
  }
  else
  {
    unsigned long lastSyncAgoSeconds = (now - lastSync) / 1000;
    unsigned long lastSyncedAt = epochTime - lastSyncAgoSeconds;
    formatEpochAsUtc(buff, lastSyncedAt);
    body += F("\"");
    body += String(buff);
    body += F("\"");
  }
  body += F(",\n");

  unsigned long upTimeMs = now;
  body += F("\"upTimeMs\": ");
  body += String(upTimeMs);
  body += F(",\n");

  body += F("\"systemBootedAt\": \"");
  unsigned long lastBoot = epochTime - (upTimeMs / 1000);
  formatEpochAsUtc(buff, lastBoot);
  body += String(buff);
  body += F("\"");

  body += F("\n}\n");

  client.print(F("Content-Length: "));
  client.println(body.length());
  client.println(); // do not forget this one

  client.print(body);
}

void outputStatusAsHtml(WiFiClient *pClient)
{
  WiFiClient client = *pClient;

  // Return the response
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(); // do not forget this one
  client.println(F("<!DOCTYPE HTML>"));
  client.println(F("<html>"));
  client.println(F("<body>"));

  client.print(F("<p>"));
  client.print(F("<a href=\"/\"><button>Home</button></a> "));
  client.print(F("<a href=\"/show\"><button>Show</button></a> "));
  client.print(F("<a href=\"/force-sync\"><button>Sync</button></a> "));
  client.print(F("<a href=\"/update-schedule\"><button>Update Schedule</button></a>"));
  client.println(F("</p>"));

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
  if (scheduleRetrieved)
  {
    client.print(F("<p>The schedule was last retrieved "));
    printTimeAgo(&client, lastScheduleUpdate);
    client.print(F("<p>"));
  }
  else
  {
    client.println(F("<p>Pending retrieval... Try again shortly.</p>"));
  }

  if (lastSync == 0)
  {
    client.println(F("<p>The clock has not been synced since boot</p>"));
  }
  else
  {
    client.print(F("<p>The clock was last synced "));
    printTimeAgo(&client, lastSync);
    client.print(F("</p>"));
  }

  client.print(F("<p>Last boot: "));
  printTimeAgo(&client, 0);
  client.print(F("</p>"));

  client.println(F("</body>"));
  client.println(F("</html>"));
}

void printTimeAgo(WiFiClient *pClient, unsigned long previousTime, bool includeAgo)
{
  WiFiClient client = *pClient;

  unsigned long ago = millis() - previousTime;

  unsigned long hoursAgo = ago / 1000 / 60 / 60;
  unsigned long minutesAgo = ago / 1000 / 60;
  unsigned long secondsAgo = ago / 1000;

  if (hoursAgo > 0)
  {
    client.print(hoursAgo);
    client.print(F(" hours"));
  }
  else if (minutesAgo > 0)
  {
    client.print(minutesAgo);
    client.print(F(" minutes"));
  }
  else
  {
    client.print(secondsAgo);
    client.print(F(" seconds"));
  }

  if (includeAgo)
  {
    client.println(F(" ago"));
  }
}

void pushUpdate()
{
  Serial.print(F(">set:"));
  Serial.print(timeClient.getEpochTime());
  Serial.println(F("!"));
  delay(300);

  for (byte i = 0; i < MAX_SCHEDULES; i++)
  {
    if (schedule[i] > 0)
    {
      Serial.print(F(">set-schedule:"));
      Serial.print(schedule[i]);
      Serial.println(F("!"));
      delay(300);
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