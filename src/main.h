#ifndef main_h
#define main_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Eventually.h>
#include <EventuallyStateMachine.h>
#include "credentials.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

int ledPin = 2;
WiFiServer server(80);
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "uk.pool.ntp.org");

EvtManager mgr;
EvtStateMachineListener stateMachine;

#endif