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

#define LED 2
#define MAX_SCHEDULES 10
#define TIME_UPDATE_SCHEDULE 1000          // 1 second
#define TIME_UPDATE_INTERVAL 1000 * 60 * 5 // 5 minutes

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *scheduleUrl = SCHEDULE_URL;
const char *scheduleSourceUrl = SCHEDULE_SOURCE_URL;

WiFiServer server(80);
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "uk.pool.ntp.org");

EvtManager mgr;
EvtCommandListener commandListener(&Serial, 20);

long schedule[MAX_SCHEDULES];

void initializeSchedule();
void pushUpdate();
bool handleWifiClient();
bool updateSchedule();
bool updateTime();
bool forceUpdate(IEvtListener *, IEvtContext *, long data);

EvtTimeListener updateTimeListener(TIME_UPDATE_SCHEDULE, true, (EvtAction)updateTime);
EvtTimeListener handleWifiClientListener(0, true, (EvtAction)handleWifiClient);

#endif