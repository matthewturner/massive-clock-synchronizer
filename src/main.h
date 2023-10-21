#ifndef main_h
#define main_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Eventually.h>
#include <EventuallyCommand.h>
#include "secrets.h"
#include <Fetch.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *scheduleUrl = SCHEDULE_URL;
const char *scheduleSourceUrl = SCHEDULE_SOURCE_URL;

WiFiServer server(80);
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "uk.pool.ntp.org");

EvtManager mgr;
EvtCommandListener commandListener(&Serial, 20);

long schedule[10];

void initializeSchedule();
void forceUpdate();
bool handleWifiClient();
bool updateSchedule();
bool updateTime();
bool forceUpdateSchedule(IEvtListener *, IEvtContext *, long data);

EvtTimeListener updateScheduleListener(30000, true, (EvtAction)updateSchedule);
EvtTimeListener updateTimeListener(1000, true, (EvtAction)updateTime);
EvtTimeListener handleWifiClientListener(0, true, (EvtAction)handleWifiClient);

#endif