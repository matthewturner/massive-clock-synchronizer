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

  // stateMachine.when(IDLE, (EvtAction)idle, UPDATING);
  // stateMachine.transition(UPDATING);

  // mgr.addListener(&stateMachine);
}

bool idle()
{
  Serial.println(F("Sleeping..."));
  Serial.flush();
  return true;
}

bool show()
{
  Serial.println(F("Command: SHOW"));
  stateMachine.transition(SHOWING);
  return true;
}

int counter = 0;

bool update()
{
  counter++;
  if (counter < 1000)
  {
    return false;
  }
  counter = 0;

  Serial.println(F("Command: UPDATE"));
  // stateMachine.transition(IDLE);
  RequestOptions options;
  options.method = "GET";
  options.headers["Connection"] = "keep-alive";
  options.headers["accept"] = "text/plain";

  Serial.println("Sending the request...");

  Response response = fetch(scheduleUrl, options);
  schedule = response.text();
  int startIndex = 0;
  int endIndex = schedule.indexOf('\r');
  Serial.println(schedule.substring(startIndex, endIndex));
  startIndex = endIndex + 1;
  endIndex = schedule.length() - 1;
  Serial.println(schedule.substring(startIndex, endIndex));
  Serial.println("Finished");

  return true;
}

void handleConnectedWifiClient()
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
  client.print("<a href=\"");
  client.print(scheduleUrl);
  client.print("\">Current Schedule</a>");
  client.println(F("</p>"));

  client.print(F("<p>"));
  client.print("<a href=\"");
  client.print(scheduleSourceUrl);
  client.print("\">Schedule Source</a>");
  client.println(F("</p>"));

  client.print(F("<p>"));
  client.print(schedule);
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

void loop()
{
  update();
  handleConnectedWifiClient();
  // mgr.loopIteration();
}