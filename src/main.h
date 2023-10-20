#ifndef main_h
#define main_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Eventually.h>
#include <EventuallyStateMachine.h>
#include "secrets.h"
#include <Fetch.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *scheduleUrl = SCHEDULE_URL;
const char *scheduleSourceUrl = SCHEDULE_SOURCE_URL;

const byte IDLE = 0;
const byte UPDATING = 1;
const byte SHOWING = 2;

int ledPin = 2;
WiFiServer server(80);
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "uk.pool.ntp.org");

EvtManager mgr;
EvtStateMachineListener stateMachine;
String schedule;

void handleConnectedWifiClient();
bool idle();
bool update();
bool show();

#endif