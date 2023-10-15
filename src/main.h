#ifndef main_h
#define main_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Eventually.h>
#include <EventuallyStateMachine.h>

const char *ssid = "replace";
const char *password = "replace";

int ledPin = 2;
WiFiServer server(80);
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "uk.pool.ntp.org");

EvtManager mgr;
EvtStateMachineListener stateMachine;

#endif